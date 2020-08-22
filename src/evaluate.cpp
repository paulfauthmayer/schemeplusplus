#include "evaluate.hpp"
#include <iostream>
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
  std::cout << "evaluate builtin function\n";
  int nArgs = evaluateArguments(env, arguments);
  switch (getBuiltinFuncTag(function)) {
    case FUNC_ADD:
      return addFunction(argumentStack, nArgs);
      break;
    case FUNC_SUB:
      return subFunction(argumentStack, nArgs);
      break;
    case FUNC_MULT:
      break;
    case FUNC_CONS:
      break;
    case FUNC_CAR:
      break;
    case FUNC_CDR:
      break;
    case FUNC_EQUAL:
      break;
    case FUNC_GT:
      break;
    case FUNC_LT:
      break;
    case FUNC_DISPLAY:
      break;
    case FUNC_LIST:
      break;

    default:
      schemeThrow("undefined builtin function: " + toString(function));
      break;
  }
}
static Object* evaluateUserDefinedFunction(Environment& env,
                                           scm::Object* function,
                                           scm::Object* arguments)
{
  std::cout << "evaluate user defined function\n";
  return SCM_NIL;
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
      std::cout << "getting variable " + toString(obj) + "\n";
      evaluatedObj = getVariable(env, obj);
      if (!evaluatedObj) {
        schemeThrow("undefined variable: " + std::get<std::string>(obj->value));
      }
      std::cout << "evaluated " + toString(obj) + " to " + toString(evaluatedObj) + '\n';
      return evaluatedObj;
      break;
    }
    case scm::TAG_CONS: {
      Object* operation{getCar(obj)};
      Object* arguments{getCdr(obj)};
      std::cout << "operation: " + toString(operation) + " args: " + toString(arguments) + "\n";
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
