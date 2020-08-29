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
  DLOG_F(ERROR, "trampoline");
  DLOG_F(WARNING, "getNextFunction");
  Continuation* nextFunction{startFunction};
  pushFunc(NULL);
  while (nextFunction != NULL) {
    DLOG_F(ERROR, "trampoline loop");
    DLOG_F(WARNING, "getNextFunction");
    DLOG_F(WARNING, "funcStack at %d", functionStack.size());
    nextFunction = (Continuation*)(*nextFunction)();
  }
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
  DLOG_F(WARNING, "evaluate expression");
  pushArgs({&env, expression});
  return trampoline(cont(evaluate));
}

// evaluate functions and syntax
// static int evaluateArguments(Environment& env, Object* arguments)
// {
//   /**
//    * Evaluates a list of arguments and stores them on the argument stack
//    */
//   DLOG_F(WARNING, "evaluate arguments: %s", toString(arguments).c_str());
//   std::size_t initialSize{argumentStack.size()};
//   while (arguments != SCM_NIL) {
//     auto currentArguement = getCar(arguments);
//     // argumentStack.push(evaluate(env, currentArguement));
//     pushArg(SCM_NIL);  // TODO!
//     arguments = getCdr(arguments);
//   }
//   return argumentStack.size() - initialSize;
// }

// forward declaration of the following functions parts
static Continuation* evaluateArguments_Part1();
/**
 * Evaluates the argument cons object that used to be passed to functions and
 * stores them in the argument stack.
 * @return the next step in our trampoline
 */
static Continuation* evaluateArguments()
{
  DLOG_F(ERROR, "evaluate arguments");
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
    return popFunc();
  }
};

static Continuation* evaluateArguments_Part1()
{
  DLOG_F(WARNING, "evaluate arguments part1");
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
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};
  printArgStack();
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
  Environment* env{popArg<Environment*>()};
  Object* function{popArg<Object*>()};
  int nArgs{static_cast<int>(argumentStack.size() - popArg<std::size_t>() - 1)};
  ObjectVec evaluatedArguments{popArgs<Object*>(nArgs)};

  DLOG_F(INFO, "evaluate user defined function");
  Object* functionArguments{getUserFunctionArgList(function)};
  Object* functionBody{getUserFunctionBodyList(function)};
  Object* lastBodyResult;
  Environment* funcEnv{new Environment(getUserFunctionParentEnv(function))};

  while (functionArguments != SCM_NIL) {
    Object* argName{getCar(functionArguments)};
    Object* argValue{evaluatedArguments[--nArgs]};
    define(*funcEnv, argName, argValue);
    functionArguments = getCdr(functionArguments);
  }

  return tCall(cont(evaluate), {funcEnv, functionBody});
}

static Continuation* evaluateSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* syntax{popArg<Object*>()};
  Object* arguments{popArg<Object*>()};
  DLOG_F(INFO, "evaluate builtin syntax %s", toString(syntax).c_str());
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
    default:
      schemeThrow("undefined syntax: " + toString(syntax));
      break;
  }
}

static Continuation* evaluate_Part1();
Continuation* evaluate()
{
  DLOG_F(WARNING, "evaluate");
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
      t_RETURN(obj);  // TODO: continue implementation here

    case scm::TAG_SYMBOL: {
      DLOG_F(INFO, "getting variable %s", toString(obj).c_str());
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
  DLOG_F(WARNING, "evaluate part1");
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
