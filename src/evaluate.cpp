#include "evaluate.hpp"
#include <iostream>
#include <loguru.hpp>
#include <stack>
#include "environment.hpp"
#include "memory.hpp"
#include "operations.hpp"
#include "scheme.hpp"

namespace scm {

// define a stack that can hold multiple objects!
static ObjectStack argumentStack{};

// evaluate functions and syntax
static int evaluateArguments(Environment& env, Object* arguments)
{
  /**
   * Evaluates a list of arguments and stores them on the argument stack
   */
  std::size_t initialSize{argumentStack.size()};
  while (arguments != SCM_NIL) {
    auto currentArguement = getCar(arguments);
    argumentStack.push(evaluate(env, currentArguement));
    arguments = getCdr(arguments);
  }
  return argumentStack.size() - initialSize;
}

static Object* evaluateBuiltinFunction(Environment& env,
                                       scm::Object* function,
                                       scm::Object* arguments)
{
  DLOG_F(INFO, "evaluate builtin function %s", getBuiltinFuncName(function).c_str());
  int nArgs = evaluateArguments(env, arguments);
  if (nArgs != getBuiltinFuncNArgs(function) && getBuiltinFuncNArgs(function) != -1) {
    schemeThrow("function " + getBuiltinFuncName(function) + " expects " +
                std::to_string(getBuiltinFuncNArgs(function)) + " arguments, got " +
                std::to_string(nArgs) + '\n');
  }
  switch (getBuiltinFuncTag(function)) {
    case FUNC_ADD:
      return addFunction(argumentStack, nArgs);
      break;
    case FUNC_SUB:
      return subFunction(argumentStack, nArgs);
      break;
    case FUNC_MULT:
      return multFunction(argumentStack, nArgs);
      break;
    case FUNC_DIV:
      return divFunction(argumentStack, nArgs);
      break;
    case FUNC_MOD:
      return modFunction(argumentStack, nArgs);
      break;
    case FUNC_CONS:
      return consFunction(argumentStack, nArgs);
      break;
    case FUNC_CAR:
      return carFunction(argumentStack, nArgs);
      break;
    case FUNC_CDR:
      return cdrFunction(argumentStack, nArgs);
      break;
    case FUNC_EQ:
      return eqFunction(argumentStack, nArgs);
      break;
    case FUNC_EQUAL:
      return equalFunction(argumentStack, nArgs);
      break;
    case FUNC_EQUAL_NUMBER:
      return equalNumberFunction(argumentStack, nArgs);
      break;
    case FUNC_GT:
      return greaterThanFunction(argumentStack, nArgs);
      break;
    case FUNC_LT:
      return lesserThanFunction(argumentStack, nArgs);
      break;
    case FUNC_DISPLAY:
      return displayFunction(argumentStack, nArgs);
      break;
    case FUNC_LIST:
      return listFunction(argumentStack, nArgs);
      break;
    case FUNC_FUNCTION_BODY:
      return functionBodyFunction(argumentStack, nArgs);
      break;
    case FUNC_FUNCTION_ARGLIST:
      return functionArglistFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_STRING:
      return isStringFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_NUMBER:
      return isNumberFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_CONS:
      return isConsFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_FUNC:
      return isBuiltinFunctionFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_USERFUNC:
      return isUserFunctionFunction(argumentStack, nArgs);
      break;
    case FUNC_IS_BOOL:
      return isBoolFunction(argumentStack, nArgs);
      break;
    default:
      schemeThrow("undefined builtin function: " + toString(function));
      break;
  }
  // this will never be returned!
  return SCM_VOID;
}

static Object* evaluateUserDefinedFunction(Environment& env,
                                           scm::Object* function,
                                           scm::Object* arguments)
{
  std::cout << "evaluate user defined function\n";
  return SCM_VOID;
}
static Object* evaluateSyntax(Environment& env, scm::Object* function, scm::Object* arguments)
{
  std::cout << "evaluate syntax\n";
  return SCM_NIL;
}

scm::Object* evaluate(Environment& env, scm::Object* obj)
{
  scm::Object* evaluatedObj;

  switch (obj->tag) {
    case scm::TAG_INT:
    case scm::TAG_FLOAT:
    case scm::TAG_STRING:
    case scm::TAG_NIL:
    case scm::TAG_FALSE:
    case scm::TAG_TRUE:
    case scm::TAG_FUNC_BUILTIN:
      return obj;
      break;

    case scm::TAG_SYMBOL: {
      DLOG_F(INFO, "getting variable %s", toString(obj).c_str());
      evaluatedObj = getVariable(env, obj);
      if (!evaluatedObj) {
        schemeThrow("undefined variable: " + std::get<std::string>(obj->value));
      }
      DLOG_F(INFO,
             "evaluated variable %s to %s",
             toString(obj).c_str(),
             toString(evaluatedObj).c_str());
      return evaluatedObj;
      break;
    }
    case scm::TAG_CONS: {
      Object* operation{getCar(obj)};
      Object* arguments{getCdr(obj)};
      DLOG_F(INFO,
             "operation: %s arguments: %s",
             toString(operation).c_str(),
             toString(arguments).c_str());
      Object* evaluatedOperation = evaluate(env, operation);
      switch (evaluatedOperation->tag) {
        case TAG_FUNC_BUILTIN:
          return evaluateBuiltinFunction(env, evaluatedOperation, arguments);
          break;
        case TAG_SYNTAX:
          return evaluateSyntax(env, evaluatedOperation, arguments);
          break;
        case TAG_FUNC_USER:
          return evaluateUserDefinedFunction(env, evaluatedOperation, arguments);
          break;
        default:
          schemeThrow(toString(evaluatedOperation) + " doesn't exist");
          break;
      }
      break;
    }
    default:
      schemeThrow("evaluation not yet implemented for " + scm::toString(obj));
  }

  return SCM_NIL;
}

}  // namespace scm
