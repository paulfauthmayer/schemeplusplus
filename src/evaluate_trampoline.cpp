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

// Macros
// we frequently need to convert a funciton Pointer to a Continuation Pointer
#define cont(x) (Continuation*)(x)

#define t_RETURN(rVal)      \
  {                         \
    lastReturnValue = rVal; \
    return popFunc();       \
  }

/**
 * This function is used as a wrapper for our trampoline in order to
 * obfuscate it from the main REPL.
 * @param env The top level environment used for the evaluation
 * @param expression The expression to be evaluated
 * @returns the result of the expression
 */
Object* evaluateExpression(Environment& env, Object* expression)
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateExpression");
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "expression: %s", toString(expression).c_str());
  pushArgs({&env, expression});
  return trampoline(cont(evaluate));
}

// evaluate functions and syntax

// forward declaration of the following functions parts
static Continuation* evaluateArguments_Part1();
/**
 * Evaluates the argument cons object that used to be passed to functions and
 * stores them in the argument stack.
 * Expects parameters as pop form the argument stack
 * @param env Environment in which to evaluate the arguments
 * @param operation the currently evaluated operation
 * @param argumentCons the lis of arguments as a cons Object
 * @returns the next step in our trampoline
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

/**
 * The continuation of evaluateArguments.
 * Is called again and again until all arguments have ben evaluated and pushed to the stack.
 * Expects parameters as pop form the argument stack
 * @param env Environment in which to evaluate the arguments
 * @param operation the currently evaluated operation
 * @param argumentCons the lis of arguments as a cons Object
 * @returns the next step in our trampoline
 */
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

/**
 * Evaluates a builtin function and writes the result to `lastReturnValue`.
 * Expects parameters as pop form the argument stack.
 * @param env Environment in which to evaluate the arguments
 * @param function the currently evaluated function
 * @param nArgs the number of Arguments to be popped from the argument stack
 * @returns the next function in the function stack
 */
static Continuation* evaluateBuiltinFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateBuiltinFunction");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};

  DLOG_IF_F(INFO,
            LOG_TRAMPOLINE_TRACE,
            "evaluate builtin function %s",
            getBuiltinFuncName(function).c_str());
  // catch wrong number of arguments
  if (nArgs != getBuiltinFuncNArgs(function) && getBuiltinFuncNArgs(function) != -1) {
    schemeThrow("function " + getBuiltinFuncName(function) + " expects " +
                std::to_string(getBuiltinFuncNArgs(function)) + " arguments, got " +
                std::to_string(nArgs) + '\n');
  }

  // push arguments required for evaluation of builtin functions
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

/**
 * Evaluate a user defined function object and writes the result to `lastReturnValue`.
 * Expects parameters as pop form the argument stack.
 * @param env Environment in which to evaluate the arguments
 * @param function the currently evaluated function
 * @param nArgs the number of Arguments to be popped from the argument stack
 * @returns the next function in the function stack
 */
static Continuation* evaluateUserDefinedFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateUserDefinedFunction");
  // pop arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};

  // get arguments and list of expressions
  Object* functionArguments{getUserFunctionArgList(function)};
  Object* functionBody{getUserFunctionBodyList(function)};
  Environment* funcEnv{new Environment(getUserFunctionParentEnv(function))};

  if (nArgs == 0 && functionArguments != SCM_NIL) {
    schemeThrow("to few arguments passed to function, type `(help fname)` for more information");
  }

  if (nArgs > 0) {
    ObjectVec evaluatedArguments{popArgs<Object*>(nArgs)};

    // define all function arguments in the function environment
    while (functionArguments != SCM_NIL) {
      if (nArgs == 0) {
        schemeThrow(
            "to few arguments passed to function, type `(help fname)` for more information");
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

/**
 * Evaluate a syntax object and writes the result to `lastReturnValue`.
 * Expects parameters as pop form the argument stack.
 * @param env Environment in which to evaluate the arguments
 * @param syntax the currently evaluated syntax
 * @param arguments the arguments passed to the syntax
 * @returns the next function in the function stack
 */
static Continuation* evaluateSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluateSyntax");
  // pop required arguments
  Environment* env{popArg<Environment*>()};
  Object* syntax{popArg<Object*>()};
  Object* arguments{popArg<Object*>()};

  // check if syntax really is a syntax
  if (!hasTag(syntax, TAG_SYNTAX)) {
    schemeThrow(toString(syntax) + " isn't a valid syntax");
  }

  // push arguments required for syntax evaluation
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
/**
 * Evaluates the next object on the stack
 * Expects parameters as pop from the argument stack.
 * @param env the environment in which tho evaluate the object
 * @param obj the object to be evaluated
 * @returns the next function on the function stack or another continuation function
 */
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
      DLOG_IF_F(INFO,
                LOG_EVALUATION,
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

/**
 * Continuation of evaluate, handles evaluation of functions and syntax.
 * Writes restul to `lastReturnValue`.
 * Expects parameters as pop from the argument stack.
 * @param env the environment in which tho evaluate the object
 * @param obj the object to be evaluated
 * @returns the next function on the function stack or another continuation function
 */
static Continuation* evaluate_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: evaluate part1");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* obj{popArg<Object*>()};

  // get previously evaluated operation
  Object* evaluatedOperation{lastReturnValue};
  Object* argumentCons{getCdr(obj)};
  DLOG_IF_F(INFO,
            LOG_EVALUATION,
            "operation: %s arguments: %s",
            toString(getCar(obj)).c_str(),
            toString(argumentCons).c_str());

  switch (evaluatedOperation->tag) {
    case TAG_FUNC_BUILTIN:
      // evaluate arguments first then continue with function evaluation
      return tCall(cont(evaluateArguments),
                   cont(evaluateBuiltinFunction),
                   {env, evaluatedOperation, argumentCons});
    case TAG_SYNTAX:
      return tCall(cont(evaluateSyntax), {env, evaluatedOperation, argumentCons});
    case TAG_FUNC_USER:
      // evaluate arguments first then continue with function evaluation
      return tCall(cont(evaluateArguments),
                   cont(evaluateUserDefinedFunction),
                   {env, evaluatedOperation, argumentCons});
    default:
      // this is effectively the same as quote!
      if (evaluatedOperation != NULL)
        t_RETURN(evaluatedOperation);
      schemeThrow(toString(evaluatedOperation) + " doesn't exist");
  }

  return NULL;
}

}  // namespace trampoline
}  // namespace scm
