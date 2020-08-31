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

Object* pop(ObjectStack& stack)
{
  /**
   * Pops and returns the topmost element of a given ObjectStack
   * @param stack the stack from which to pop
   * @returns the popped scm::Object*
   */
  if (stack.empty()) {
    schemeThrow("trying to pop from empty stack");
  }
  Object* obj{stack.top()};
  DLOG_F(INFO,
         "popped {%s}, %d values remain on stack. next: %s",
         toString(obj).c_str(),
         stack.size(),
         toString(stack.top()).c_str());
  stack.pop();
  return obj;
}

ObjectVec popN(ObjectStack& stack, int n)
{
  /**
   * Pops and returns the topmost N elements of a given ObjectStack
   * @param stack the stack from which to pop
   * @param n the amount of values popped
   * @returns the popped objects in a ObjectVec
   */
  DLOG_F(INFO, "popping %d values from stack", n);
  if (stack.size() < n)
    schemeThrow("stack doesn't contain " + std::to_string(n) + " arguments!");
  ObjectVec values;
  for (int i{0}; i < n; i++) {
    values.push_back(pop(stack));
  }
  return values;
}

inline void push(ObjectStack& stack, Object* obj)
{
  stack.push(obj);
}

void push(ObjectStack& stack, ObjectVec objects)
{
  for (auto i = objects.rbegin(); i != objects.rend(); i++) {
    push(stack, *i);
  }
}

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
                                           scm::Object* argumentCons)
{
  DLOG_F(INFO, "evaluate user defined function");
  Object* functionArguments{getUserFunctionArgList(function)};
  Object* functionBody{getUserFunctionBodyList(function)};
  Object* lastBodyResult;
  Environment funcEnv{getUserFunctionParentEnv(function)};

  int nArgs = evaluateArguments(env, argumentCons);
  ObjectVec evaluatedArguments{popN(argumentStack, nArgs)};
  while (functionArguments != SCM_NIL) {
    Object* argName{getCar(functionArguments)};
    Object* argValue{evaluatedArguments[--nArgs]};
    define(funcEnv, argName, argValue);
    functionArguments = getCdr(functionArguments);
  }

  return evaluate(funcEnv, functionBody);
}

static Object* evaluateSyntax(Environment& env, scm::Object* syntax, scm::Object* arguments)
{
  DLOG_F(INFO, "evaluate builtin syntax %s", toString(syntax).c_str());
  if (!hasTag(syntax, TAG_SYNTAX)) {
    schemeThrow(toString(syntax) + " isn't a valid syntax");
  }
  switch (getBuiltinFuncTag(syntax)) {
    case SYNTAX_QUOTE:
      return quoteSyntax(arguments);
      break;
    case SYNTAX_LAMBDA:
      return lambdaSyntax(env, arguments);
      break;
    case SYNTAX_DEFINE:
      return defineSyntax(env, arguments);
      break;
    case SYNTAX_IF:
      return ifSyntax(env, arguments);
      break;
    case SYNTAX_SET:
      return setSyntax(env, arguments);
      break;
    case SYNTAX_BEGIN:
      return beginSyntax(env, arguments);
      break;
    default:
      schemeThrow("undefined syntax: " + toString(syntax));
      break;
  }
  return SCM_VOID;
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
