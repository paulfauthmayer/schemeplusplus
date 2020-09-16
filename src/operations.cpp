#include "operations.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <loguru.hpp>
#include <numeric>
#include <stack>
#include <string>
#include <variant>
#include <vector>
#include "evaluate.hpp"
#include "memory.hpp"
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

// forward declaration of continuation parts
static Continuation* beginSyntax_Part1();
static Continuation* defineSyntax_Part1();
static Continuation* ifSyntax_Part1();
static Continuation* setSyntax_Part1();
static Continuation* divFunction_Part1();

// BUILTIN SYNTAX

/**
 * Prints the help text of a builtin function or syntax
 * or the formatted code of a user define function. Prints the
 * value of other objects. If no argument is specified, show all
 * bindings of the current environment.
 * Expects parameters from the argument Stack.
 * @param env environment from which to get the bindings
 * @param argumentCons the object the user requests help for
 * @returns VOID
 */
Continuation* helpSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: helpSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object* variable;
  switch (argumentCons->tag) {
    case TAG_NIL:
      printEnv(*env);
      break;
    case TAG_CONS:
      switch (getCar(argumentCons)->tag) {
        case TAG_SYMBOL: {
          variable = getVariable(*env, getCar(argumentCons));
          std::cout << "======== " << toString(getCar(argumentCons)) << " ========\n";
          switch (variable->tag) {
            case TAG_FUNC_BUILTIN:
            case TAG_SYNTAX:
              std::cout << getBuiltinFuncHelpText(variable) << '\n';
              break;
            case TAG_FUNC_USER:
              std::cout << prettifyUserFunction(variable);
              break;

            default:
              std::cout << toString(variable) << '\n';
              break;
          }
          break;
        }
        default:
          std::cout << toString(getCar(argumentCons)) << '\n';
          break;
      }
      break;
    default:
      std::cout << "4\n";
      std::cout << toString(argumentCons) << '\n';
      t_RETURN(argumentCons) break;
  }
  t_RETURN(SCM_VOID);
}

/**
 * Define a new variable in a given environment.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to define the variable
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns continuation to either defineSyntax_Part1 or defineLambda
 */
Continuation* defineSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: defineSyntax");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};

  Object *symbol, *value;

  if (argumentCons == SCM_NIL) {
    schemeThrow("define takes exactyly 2 arguments");
  }
  // get key of definition
  symbol = getCar(argumentCons);
  if (!isOneOf(symbol, {TAG_SYMBOL, TAG_CONS})) {
    schemeThrow("can only define symbols or functions");
  }

  // shorthand lambda definition!
  // if symbol is a cons, treat it like a lambda declaration
  // (define (funcname var1 var2 ...) (body))
  if (hasTag(symbol, TAG_CONS)) {
    // push arguments required for next part
    pushArgs({env, getCar(symbol)});
    // call lambda then continue with next part
    return tCall(cont(lambdaSyntax),
                 cont(defineSyntax_Part1),
                 {env, newCons(getCdr(symbol), getCdr(argumentCons))});
  }
  // in the case of a single symbol, we just define the variable
  else {
    value = getCdr(argumentCons);
    if (value == SCM_NIL || getCdr(value) != SCM_NIL) {
      schemeThrow("define takes exactyly 2 arguments");
    }
    // push arguments required for next part
    pushArgs({env, symbol});

    // call evaluate then continue with next part
    return tCall(cont(evaluate), cont(defineSyntax_Part1), {env, getCar(value)});
  }
}

/**
 * Continuation of defineSyntax, does the actual defining
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to define the variable
 * @param symbol: the key of the definition
 * @returns VOID
 */
static Continuation* defineSyntax_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: defineSyntax Part1");
  // get arguments from argument stack
  Environment* env{popArg<Environment*>()};
  Object* symbol{popArg<Object*>()};
  // get evaluated value of definition
  Object* value{lastReturnValue};

  define(*env, symbol, value);
  t_RETURN(SCM_VOID)
}

/**
 * Set a new variable in a given environment and all of its parents.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns continuation to setSyntax_Part1
 */
Continuation* setSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: setSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};

  Object *symbol, *expression;

  try {
    symbol = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    expression = getCar(argumentCons);
    if (getCdr(argumentCons) != SCM_NIL) {
      schemeThrow("set requires exactly two arguments: (set! {name} {value})");
    }
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("set requires exactly two arguments: (set! {name} {value})");
  }
  // push arguments required for next part
  pushArgs({env, symbol});

  // call evaluate then continue with next part
  return tCall(cont(evaluate), cont(setSyntax_Part1), {env, expression});
}

/**
 * Continuation of setSyntax, does the actual setting
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param symbol: the key of the definition
 * @returns VOID
 */
static Continuation* setSyntax_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: setSyntax_Part1");
  Environment* env{popArg<Environment*>()};
  Object* symbol{popArg<Object*>()};
  Object* value{lastReturnValue};

  set(*env, symbol, value);
  t_RETURN(value);
}

/**
 * Return the unevaluated first argument of the expression.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns the first argument
 */
Continuation* quoteSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: quoteSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object* quoted = (hasTag(argumentCons, TAG_CONS)) ? getCar(argumentCons) : argumentCons;
  t_RETURN(quoted);
}

/**
 * Return the first expression if a condition is true, second one otherwise.
 * Expects arguments as pop from argument stack.
 * This part handles the evaluation of the condition.
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns one of the expressions
 */
Continuation* ifSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: ifSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *condition, *trueExpression, *falseExpression;

  // get all required Objects
  try {
    condition = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    trueExpression = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    falseExpression = getCar(argumentCons);
    if (getCdr(argumentCons) != SCM_NIL) {
      schemeThrow("if requires 3 arguments: (if {condition} {true} {false})");
    }
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("if requires 3 arguments: (if {condition} {true} {false})");
  }
  // push arguments required for next part
  pushArgs({env, trueExpression, falseExpression});

  // call evaluate then continue with next part
  return tCall(cont(evaluate), cont(ifSyntax_Part1), {env, condition});
}

/**
 * Continuation of ifSyntax. Returns the correct expression based on the value
 * of the previously evaluated condition.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param symbol: the key of the definition
 * @returns VOID
 */
static Continuation* ifSyntax_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: ifSyntax Part1");
  Environment* env{popArg<Environment*>()};
  Object* trueExpression{popArg<Object*>()};
  Object* falseExpression{popArg<Object*>()};

  Object* evaluatedCondition{lastReturnValue};

  Object* conditionAsBool;
  switch (evaluatedCondition->tag) {
    case scm::TAG_INT: {
      conditionAsBool = (getIntValue(evaluatedCondition) != 0) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_FLOAT: {
      conditionAsBool = (getFloatValue(evaluatedCondition) != 0) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_STRING: {
      conditionAsBool =
          (getStringValue(evaluatedCondition) == std::string{}) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_TRUE:
    case scm::TAG_FUNC_BUILTIN:
    case scm::TAG_FUNC_USER:
    case scm::TAG_SYNTAX: {
      conditionAsBool = SCM_TRUE;
      break;
    }
    case scm::TAG_NIL:
    case scm::TAG_FALSE: {
      conditionAsBool = SCM_FALSE;
      break;
    }
    default: {
      schemeThrow("evaluation not yet implemented for " + toString(evaluatedCondition) +
                  " with tag " + tagToString(evaluatedCondition->tag));
      break;
    }
  }
  Object* expression{(conditionAsBool == SCM_TRUE) ? trueExpression : falseExpression};
  return tCall(cont(evaluate), {env, expression});
}

/**
 * Evaluate a body of expressions and return the last result.
 * Expects arguments as pop from argument stack.
 * This part only starts the process!
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns one of the expressions
 */
Continuation* beginSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: beginSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  // because of this check we need to split the function
  // SCM_NIL is required as check for the end of cons objects
  if (argumentCons == SCM_NIL) {
    t_RETURN(SCM_VOID);
  }
  else {
    return tCall(cont(beginSyntax_Part1), {env, argumentCons});
  }
}

/**
 * Continuation of beginSyntax, does the actual evaluation and loop.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @returns one of the expressions
 */
static Continuation* beginSyntax_Part1()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: beginSyntax Part1");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};

  Object* currentExpression = getCar(argumentCons);
  argumentCons = getCdr(argumentCons);

  // check if we're finished
  if (argumentCons == SCM_NIL) {
    // evaluate last expression in begin block and return
    return tCall(cont(evaluate), {env, currentExpression});
  }
  else {
    // push arguments for next part
    pushArgs({env, argumentCons});
    return tCall(cont(evaluate), cont(beginSyntax_Part1), {env, currentExpression});
  }
}

/**
 * Create a new user defined function.
 * Expects arguments as pop from argument stack.
 * @param env: the environment in which to start
 * @param argumentCons: the arguments of the operation as a cons object
 * @see defineSyntax
 * @returns one of the expressions
 */
Continuation* lambdaSyntax()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: lambdaSyntax");
  // get arguments from stack
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};

  // try to get argument list and body list from arguments
  Object *argList, *bodyList;
  try {
    argList = getCar(argumentCons);
    bodyList = getCdr(argumentCons);
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("lambda requires at least two arguments: (lambda {argument} {body})");
  }
  t_RETURN(newUserFunction(argList, bodyList, *env));
}

// BUILTIN FUNCTIONS

/**
 * Function that handles the addition or concatenation of multiple scm::Objects
 * @param nArgs: how many arguments the function should take from the stack
 * @returns a new scm::Object* with the result of the computation
 */
Continuation* addFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: addFunction");
  int nArgs{popArg<int>()};
  DLOG_IF_F(INFO, LOG_STACK_TRACE, "nArgs = %d", nArgs);

  // get all arguments necessary and check for type validity
  if (nArgs <= 0) {
    schemeThrow("expected at least 1 argument");
  }
  auto arguments = popArgs<Object*>(nArgs);
  auto isValidType = [](Object* obj) { return isOneOf(obj, {TAG_INT, TAG_FLOAT, TAG_STRING}); };
  if (!std::all_of(arguments.begin(), arguments.end(), isValidType)) {
    schemeThrow("invalid types for add function!\n");
  }

  // this function is heavily overloaded, as it should work with strings and numeric values
  // in case any argument is a string, we handle all values as string
  // else if any are floats, we handle these as float
  // defaults to integers

  // case: at least one string
  if (std::any_of(arguments.begin(), arguments.end(), isString)) {
    auto lambda = [](std::string a, Object* b) {
      if (hasTag(b, TAG_STRING)) {
        return getStringValue(b) + a;
      }
      else if (hasTag(b, TAG_FLOAT)) {
        return std::to_string(getFloatValue(b)) + a;
      }
      else {
        return std::to_string(getIntValue(b)) + a;
      }
    };
    std::string result = std::accumulate(arguments.begin(), arguments.end(), std::string{}, lambda);
    t_RETURN(newString(result));
  }

  // case: at least one float
  else if (std::any_of(arguments.begin(), arguments.end(), isFloatingPoint)) {
    auto lambda = [](double a, Object* b) {
      if (hasTag(b, TAG_FLOAT)) {
        return getFloatValue(b) + a;
      }
      else {
        return static_cast<double>(getIntValue(b) + a);
      }
    };
    double result = std::accumulate(arguments.begin(), arguments.end(), double(0.0), lambda);
    t_RETURN(newFloat(result));
  }

  // case: all are integers
  else {
    auto lambda = [](int a, Object* b) {
      int result = getIntValue(b) + a;
      if (a > 0 && getIntValue(b) > 0 and (result < a || result < getIntValue(b))) {
        DLOG_F(ERROR, "integer overflow detected!");
        schemeThrow("integer overflow detected!");
      }
      return result;
    };
    double result = std::accumulate(arguments.begin(), arguments.end(), 0, lambda);
    t_RETURN(newInteger(result));
  }
}

/**
 * Function that handles the subtraction of one or more Objects.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns a new scm::Object* with the result of the computation
 */
Continuation* subFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: subFunction");
  int nArgs{popArg<int>()};
  auto subtrahends = popArgs<Object*>(nArgs - 1);
  int intSubtrahend{};
  double doubleSubtrahend;
  // TODO: this is a really ugly hack, fix this! it works, but gives the option for floating point
  // errors
  Object* minuendObj = popArg<Object*>();
  double minuend = hasTag(minuendObj, TAG_FLOAT) ? getFloatValue(minuendObj)
                                                 : static_cast<double>(getIntValue(minuendObj));

  if (subtrahends.size() == 0) {
    if (hasTag(minuendObj, TAG_FLOAT))
      t_RETURN(newFloat(-getFloatValue(minuendObj)));
    t_RETURN(newInteger(-getIntValue(minuendObj)));
  }
  else if (hasTag(minuendObj, TAG_FLOAT) ||
           std::any_of(subtrahends.begin(), subtrahends.end(), isFloatingPoint)) {
    auto lambda = [](double a, Object* b) {
      if (hasTag(b, TAG_INT)) {
        return a + static_cast<double>(getIntValue(b));
      }
      return a + getFloatValue(b);
    };
    doubleSubtrahend = std::accumulate(subtrahends.begin(), subtrahends.end(), double(0.0), lambda);
    t_RETURN(newFloat(minuend - doubleSubtrahend));
  }
  else {
    auto lambda = [](int a, Object* b) { return a + getIntValue(b); };
    intSubtrahend = std::accumulate(subtrahends.begin(), subtrahends.end(), int(0), lambda);
    t_RETURN(newInteger(static_cast<int>(minuend) - intSubtrahend));
  }
}

/**
 * function that handles the multiplication of one or more numeric objects.
 * @param nargs: how many arguments the function should take from the stack
 * @returns a new scm::object* with the result of the computation
 */
Continuation* multFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: multFunction");
  int nArgs{popArg<int>()};
  ObjectVec arguments{popArgs<Object*>(nArgs)};
  auto isValidType = [](Object* obj) { return isOneOf(obj, {TAG_INT, TAG_FLOAT}); };
  if (!std::all_of(arguments.begin(), arguments.end(), isValidType)) {
    schemeThrow("invalid type for multiplication");
  }
  else if (std::any_of(arguments.begin(), arguments.end(), isFloatingPoint)) {
    auto lambda = [](double a, Object* b) {
      if (hasTag(b, TAG_INT)) {
        return static_cast<double>(getIntValue(b)) * a;
      }
      return a * getFloatValue(b);
    };
    t_RETURN(newFloat(std::accumulate(arguments.begin(), arguments.end(), double(1), lambda)));
  }
  else {
    auto lambda = [](int a, Object* b) {
      int result = getIntValue(b) * a;
      if (a > 0 && getIntValue(b) > 0 and (result < a || result < getIntValue(b))) {
        DLOG_F(ERROR, "integer overflow detected!");
        schemeThrow("integer overflow detected!");
      }
      return a * getIntValue(b);
    };
    t_RETURN(newInteger(std::accumulate(arguments.begin(), arguments.end(), int{1}, lambda)));
  }
}

/**
 * Function that handles the division of an Object by the product of one or more numeric Objects.
 * This part calls the evaluation of the divisor.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns a continuation to multFunction and divFunction_Part1
 */
Continuation* divFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: divFunction");
  int nArgs{popArg<int>()};
  if (nArgs < 2) {
    schemeThrow("division needs at least 2 arguments");
  }
  return tCall(cont(multFunction), cont(divFunction_Part1), {nArgs - 1});
}

/**
 * Continuation of divFunction. Gets evaluated divisor and applys division.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns a new scm::Object* with the result of the computation
 */
Continuation* divFunction_Part1()
{
  Object* divisor{lastReturnValue};
  Object* dividend{popArg<Object*>()};

  if (isFloatingPoint(dividend) && isFloatingPoint(divisor)) {
    t_RETURN(newFloat(getFloatValue(dividend) / getFloatValue(divisor)));
  }
  else if (isFloatingPoint(dividend)) {
    t_RETURN(newFloat(getFloatValue(dividend) / static_cast<double>(getIntValue(divisor))));
  }
  else if (isFloatingPoint(divisor)) {
    t_RETURN(newFloat(static_cast<double>(getIntValue(dividend)) / getFloatValue(divisor)));
  }
  else {
    t_RETURN(newFloat(static_cast<double>(getIntValue(dividend)) /
                      static_cast<double>(getIntValue(divisor))));
  }
  t_RETURN(NULL);
}

/**
 * Call a given function with the specified arguments
 * @param nArgs: how many arguments the function should take from the stack
 * @returns the return value of the called function
 */
Object* applyFunction()
{
  Object *function, *argumentCons;
  return SCM_NIL;
}

/**
 * Perform pointer comparison of two objects.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_False dependent on result
 */
Continuation* eqFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: eqFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  t_RETURN((a == b) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks if the string values of two string objects are the same.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_False dependent on result
 */
Continuation* equalStringFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: equalStringFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  if (!isString(a) || !isString(b)) {
    schemeThrow("equal-string? only works with strings");
  }
  else {
    t_RETURN((getStringValue(a) == getStringValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
}

/**
 * Checks if the numeric values of two Objects are the same.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_False dependent on result
 */
Continuation* equalNumberFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: equalNumberFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    t_RETURN((getFloatValue(a) == getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(a)) {
    t_RETURN((getFloatValue(a) == getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(b)) {
    t_RETURN((getIntValue(a) == getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else {
    t_RETURN((getIntValue(a) == getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
}

/**
 * Checks if the numeric values of the first object is greater than that of the scond.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_False dependent on result
 */
Continuation* greaterThanFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: greaterThanFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    t_RETURN((getFloatValue(a) > getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(a)) {
    t_RETURN((getFloatValue(a) > getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(b)) {
    t_RETURN((getIntValue(a) > getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else {
    t_RETURN((getIntValue(a) > getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
}

/**
 * Checks if the numeric values of the first object is lesser than that of the scond.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_False dependent on result
 */
Continuation* lesserThanFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: lesserThanFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    t_RETURN((getFloatValue(a) < getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(a)) {
    t_RETURN((getFloatValue(a) < getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else if (isFloatingPoint(b)) {
    t_RETURN((getIntValue(a) < getFloatValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
  else {
    t_RETURN((getIntValue(a) < getIntValue(b)) ? SCM_TRUE : SCM_FALSE);
  }
}

/**
 * Constructs a cons object out of a passed car and cdr.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns the new cons object
 */
Continuation* consFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: consFunction");
  int nArgs{popArg<int>()};
  Object* cdr{popArg<Object*>()};
  Object* car{popArg<Object*>()};
  t_RETURN(newCons(car, cdr));
}

/**
 * Gets the car of a specified cons object
 * @param nArgs: how many arguments the function should take from the stack
 * @returns the car object
 */
Continuation* carFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: carFunction");
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get car value from non-cons object");
  }
  t_RETURN(getCar(cons));
}

/**
 * Gets the cdr of a specified cons object
 * @param nArgs: how many arguments the function should take from the stack
 * @returns the cdr object
 */
Continuation* cdrFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: cdrFunction");
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get cdr value from non-cons object");
  }
  t_RETURN(getCdr(cons));
}

/**
 * Creates a new cons object from the passed arguments.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns the cons object
 */
Continuation* listFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: listFunction");
  int nArgs{popArg<int>()};
  Object* rest = SCM_NIL;
  while (nArgs--) {
    Object* currentArgument{popArg<Object*>()};
    rest = newCons(currentArgument, rest);
  }
  t_RETURN(rest);
}

/**
 * Prints the passed argument to the console.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns VOID
 */
Continuation* displayFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: displayFunction");
  int nArgs{popArg<int>()};
  ObjectVec arguments{popArgs<Object*>(nArgs)};
  for (auto argument{arguments.rbegin()}; argument != arguments.rend(); argument++) {
    std::cout << toString(*argument) << " ";
  }
  std::cout << '\n';
  t_RETURN(SCM_VOID);
}

/**
 * Returns the function body of the passed user defined function.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns VOID
 */
Continuation* functionBodyFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: functionBodyFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function body is not a lambda");
  }
  t_RETURN(getUserFunctionBodyList(obj));
}

/**
 * Returns the argument list of the passed user defined function.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns VOID
 */
Continuation* functionArglistFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: functionArglistFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function arglist is not a lambda");
  }
  t_RETURN(getUserFunctionArgList(obj));
}

/**
 * Checks whether the argument is a string.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isStringFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isStringFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isString(obj)) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks whether the argument is a number.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isNumberFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isNumberFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isNumeric(obj)) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks whether the argument is a cons object.
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isConsFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isConsFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_CONS)) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks whether the argument is a builtin function
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isBuiltinFunctionFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isBuiltinFunctionFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_BUILTIN)) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks whether the argument is a user defined function
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isUserFunctionFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isUserFunctionFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_USER)) ? SCM_TRUE : SCM_FALSE);
}

/**
 * Checks whether the argument is a user real schem boolean
 * (i.e. not 1 , "asd" or + but #t or #f).
 * @param nArgs: how many arguments the function should take from the stack
 * @returns SCM_TRUE or SCM_FALSE
 */
Continuation* isBoolFunction()
{
  DLOG_IF_F(INFO, LOG_TRAMPOLINE_TRACE, "in: isBoolFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isOneOf(obj, {TAG_TRUE, TAG_FALSE})) ? SCM_TRUE : SCM_FALSE);
}

}  // namespace trampoline
}  // namespace scm