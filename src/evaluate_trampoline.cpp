#include "evaluate_trampoline.hpp"
#include <functional>
#include <iostream>
#include <loguru.hpp>
#include <stack>
#include "environment.hpp"
#include "memory.hpp"
#include "operations_trampoline.hpp"
#include "scheme.hpp"
#include "trampoline.hpp"

namespace scm {
namespace trampoline {

// TODO: move them to a common place
// Macros
// we frequently need to convert a funciton Pointer to a Continuation Pointer
#define cont(x) (Continuation*)(x)

#define t_RETURN(rVal)      \
  {                         \
    lastReturnValue = rVal; \
    return popFunc();       \
  }

/**
 * This starts our function trampoline, which is done as a means of tail call optimization.
 * Instead of calling functinons recursively, we push them to a stack of function pointers,
 * which is then worked through one after another.
 * @param startFunction the first function of our trampoline
 * @result returns the last value returned by one of the called functions
 */
Object* trampoline(Continuation* startFunction)
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: trampoline");
  Continuation* nextFunction{startFunction};
  pushFunc(NULL);
  while (nextFunction != NULL) {
    DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: trampoline loop");
    nextFunction = (Continuation*)(*nextFunction)();
  }
  DLOG_IF_F(INFO,
            LOG_TRAMPOLINE_TRACE || LOG_STACK_TRACE,
            "trampoline finished | returning %s | argStack: %d | funcStack: %d",
            toString(lastReturnValue).c_str(),
            static_cast<int>(argumentStack.size()),
            static_cast<int>(functionStack.size()));
  return lastReturnValue;
}

/**
 * This function is used as a wrapper for our trampoline in order to
 * obfuscate it from the main REPL.
 * @param env The top level environment used for the evaluation
 * @param expression The expression to be evaluated
 * @return the result of the expression
 */
Object* evaluateExpression(Environment& env, Object* expression)
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateExpression");
  pushArgs({&env, expression});
  return trampoline(cont(evaluate));
}

// evaluate functions and syntax

// forward declaration of the following functions parts
static Continuation* evaluateArguments_Part1();
/**
 * Evaluates the argument cons object that used to be passed to functions and
 * stores them in the argument stack.
 * @return the next step in our trampoline
 */
static Continuation* evaluateArguments()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateArguments");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* operation{popArg<Object*>()};
  Object* argumentCons{popArg<Object*>()};

  // keep track of how many arguments were evaluated
  int argCount;
  std::size_t stackSizeAtStart{argumentStack.size()};

  if (argumentCons != SCM_NIL) {
    // push arguments for evaluateArgunents_Part1
    pushArgs({
        env,
        operation,
        argumentCons,     // function arguments
        stackSizeAtStart  // local variables
    });
    // push and call evaluate on current argument
    Object* currentArgument{getCar(argumentCons)};
    return tCall(cont(evaluate), cont(evaluateArguments_Part1), {env, currentArgument});
  }
  else {
    // todo: do we need to push stacksizeatstart here?
    pushArgs({env, operation, stackSizeAtStart});
    return popFunc();
  }
};

static Continuation* evaluateArguments_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateArguments Part1");
  // get variables from stack
  Environment* env = popArg<Environment*>();
  Object* operation = popArg<Object*>();
  Object* argumentCons = popArg<Object*>();
  std::size_t stackSizeAtStart = popArg<std::size_t>();

  // get evaluated object and store on stack for later functions
  pushArg(lastReturnValue);

  // "loop" with next argument or return
  argumentCons = getCdr(argumentCons);
  if (argumentCons != SCM_NIL) {
    Object* nextArg{getCar(argumentCons)};
    // arguments for evaluateArguments_Part1
    pushArgs({env, operation, argumentCons, stackSizeAtStart});
    // call evaluation for next argument
    return tCall(cont(evaluate), cont(evaluateArguments_Part1), {env, nextArg});
  }
  else {
    pushArgs({env, operation, stackSizeAtStart});
    return popFunc();
  }
};

static Continuation* evaluateBuiltinFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateBuiltinFunction");
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};
  DLOG_F(INFO, "evaluate builtin function %s", getBuiltinFuncName(function).c_str());
  if (nArgs != getBuiltinFuncNArgs(function) && getBuiltinFuncNArgs(function) != -1) {
    schemeThrow("function " + getBuiltinFuncName(function) + " expects " +
                std::to_string(getBuiltinFuncNArgs(function)) + " arguments, got " +
                std::to_string(nArgs) + '\n');
  }
  // }
  // static Continuation* evaluateBuiltinFunction_Part1(){
  pushArg(nArgs);
  switch (getBuiltinFuncTag(function)) {
    case FUNC_ADD:
      return addFunction();
      break;
    case FUNC_SUB:
      return subFunction();
      break;
    case FUNC_MULT:
      return multFunction();
      break;
    case FUNC_DIV:
      return divFunction();
      break;
    case FUNC_MOD:
      return modFunction();
      break;
    case FUNC_CONS:
      return consFunction();
      break;
    case FUNC_CAR:
      return carFunction();
      break;
    case FUNC_CDR:
      return cdrFunction();
      break;
    case FUNC_EQ:
      return eqFunction();
      break;
    // case FUNC_EQUAL:
    //   return equalFunction();
    //   break;
    case FUNC_EQUAL_STRING:
      return equalStringFunction();
      break;
    case FUNC_EQUAL_NUMBER:
      return equalNumberFunction();
      break;
    case FUNC_GT:
      return greaterThanFunction();
      break;
    case FUNC_LT:
      return lesserThanFunction();
      break;
    case FUNC_DISPLAY:
      return displayFunction();
      break;
    case FUNC_LIST:
      return listFunction();
      break;
    case FUNC_FUNCTION_BODY:
      return functionBodyFunction();
      break;
    case FUNC_FUNCTION_ARGLIST:
      return functionArglistFunction();
      break;
    case FUNC_IS_STRING:
      return isStringFunction();
      break;
    case FUNC_IS_NUMBER:
      return isNumberFunction();
      break;
    case FUNC_IS_CONS:
      return isConsFunction();
      break;
    case FUNC_IS_FUNC:
      return isBuiltinFunctionFunction();
      break;
    case FUNC_IS_USERFUNC:
      return isUserFunctionFunction();
      break;
    case FUNC_IS_BOOL:
      return isBoolFunction();
      break;
    default:
      schemeThrow("undefined builtin function: " + toString(function));
      break;
  }
}

static Continuation* evaluateUserDefinedFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateUserDefinedFunction");
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};

  Object* functionArguments{getUserFunctionArgList(function)};
  Object* functionBody{getUserFunctionBodyList(function)};
  Environment* funcEnv{new Environment(getUserFunctionParentEnv(function))};

  if (nArgs > 0) {
    ObjectVec evaluatedArguments{popArgs<Object*>(nArgs)};

    while (functionArguments != SCM_NIL) {
      if (nArgs == 0) {
        schemeThrow("to few arguments passed to function");
      }
      Object* argName{getCar(functionArguments)};
      Object* argValue{evaluatedArguments[--nArgs]};
      define(*funcEnv, argName, argValue);
      functionArguments = getCdr(functionArguments);
    }
  }

  // body may be a single expression or multiple!
  if (hasTag(getCar(functionBody), TAG_CONS)) {
    return tCall(cont(beginSyntax), {funcEnv, functionBody});
  }
  else {
    return tCall(cont(evaluate), {funcEnv, functionBody});
  }
}

static Continuation* evaluateSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateSyntax");
  Environment* env{popArg<Environment*>()};
  Object* syntax{popArg<Object*>()};
  Object* arguments{popArg<Object*>()};
  if (!hasTag(syntax, TAG_SYNTAX)) {
    schemeThrow(toString(syntax) + " isn't a valid syntax");
  }
  pushArgs({env, arguments});
  switch (getBuiltinFuncTag(syntax)) {
    case SYNTAX_QUOTE:
      return quoteSyntax();
      break;
    case SYNTAX_LAMBDA:
      return lambdaSyntax();
      break;
    case SYNTAX_DEFINE:
      return defineSyntax();
      break;
    case SYNTAX_IF:
      return ifSyntax();
      break;
    case SYNTAX_SET:
      return setSyntax();
      break;
    case SYNTAX_BEGIN:
      return beginSyntax();
      break;
    case SYNTAX_HELP:
      return helpSyntax();
    default:
      schemeThrow("undefined syntax: " + toString(syntax));
      break;
  }
}

static Continuation* evaluate_Part1();
Continuation* evaluate()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluate");
  // get current environment and expression from their stacks
  Environment* env{popArg<Environment*>()};
  Object* obj{popArg<Object*>()};

  scm::Object* evaluatedObj;
  switch (obj->tag) {
    case scm::TAG_INT:
    case scm::TAG_FLOAT:
    case scm::TAG_STRING:
    case scm::TAG_NIL:
    case scm::TAG_FALSE:
    case scm::TAG_TRUE:
    case scm::TAG_FUNC_BUILTIN:
    case scm::TAG_EOF:
      t_RETURN(obj);  // TODO: continue implementation here

    case scm::TAG_SYMBOL: {
      evaluatedObj = getVariable(*env, obj);
      if (!evaluatedObj) {
        schemeThrow("undefined variable: " + std::get<std::string>(obj->value));
      }
      DLOG_F(INFO,
             "evaluated variable %s to %s",
             toString(obj).c_str(),
             toString(evaluatedObj).c_str());
      t_RETURN(evaluatedObj);
    }
    case scm::TAG_CONS: {
      Object* operation{getCar(obj)};
      // reason for split: Object* evaluatedOperation = evaluate(env, operation);
      // push arguments for evaluate_Part1
      pushArgs({env, obj});
      // push arguments for evaluate and return
      // this evaluates the operation and finally stores it in the return value container
      return tCall(cont(evaluate), cont(evaluate_Part1), {env, operation});
    }
    default:
      schemeThrow("evaluation not yet implemented for " + scm::toString(obj));
  }
}

// TODO: comments
static Continuation* evaluate_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluate part1");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* obj{popArg<Object*>()};

  // get previously evaluated operation
  Object* evaluatedOperation{lastReturnValue};
  Object* argumentCons{getCdr(obj)};
  DLOG_F(INFO,
         "operation: %s arguments: %s",
         toString(getCar(obj)).c_str(),
         toString(argumentCons).c_str());

  switch (evaluatedOperation->tag) {
    case TAG_FUNC_BUILTIN:
      return tCall(cont(evaluateArguments),
                   cont(evaluateBuiltinFunction),
                   {env, evaluatedOperation, argumentCons});
    case TAG_SYNTAX:
      return tCall(cont(evaluateSyntax), {env, evaluatedOperation, argumentCons});
    case TAG_FUNC_USER:
      return tCall(cont(evaluateArguments),
                   cont(evaluateUserDefinedFunction),
                   {env, evaluatedOperation, argumentCons});
    default:
      schemeThrow(toString(evaluatedOperation) + " doesn't exist");
      break;
  }

  return NULL;
}

}  // namespace trampoline
}  // namespace scm
