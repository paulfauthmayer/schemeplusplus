#include "operations_trampoline.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <loguru.hpp>
#include <numeric>
#include <stack>
#include <string>
#include <variant>
#include <vector>
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
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

// HELPER FUNCTIONS
Object* toSchemeBool(Environment& env, Object* evaluatedObject)
{
  // TODO: maybe move this out, don't require env but require evaluated Object
  switch (evaluatedObject->tag) {
    case scm::TAG_INT: {
      return (getIntValue(evaluatedObject) != 0) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_FLOAT: {
      return (getFloatValue(evaluatedObject) != 0) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_STRING: {
      return (getStringValue(evaluatedObject) == std::string{}) ? SCM_TRUE : SCM_FALSE;
      break;
    }
    case scm::TAG_TRUE:
    case scm::TAG_FUNC_BUILTIN:
    case scm::TAG_FUNC_USER:
    case scm::TAG_SYNTAX: {
      return SCM_TRUE;
      break;
    }
    case scm::TAG_NIL:
    case scm::TAG_FALSE: {
      return SCM_FALSE;
      break;
    }
    default: {
      schemeThrow("evaluation not yet implemented for " + scm::toString(evaluatedObject));
      break;
    }
  }
  return SCM_FALSE;
}

// BUILTIN SYNTAX
Continuation* defineSyntax()
{
  // Environment& env, scm::Object* arguments
  /**
   * Define a new variable in a given environment
   * @param env: the environment in which to define the variable
   * @param argumentCons: the arguments of the operation as a cons object
   * @return a scm::Object with the result of the operation
   */
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *symbol, *value;
  if (argumentCons == SCM_NIL) {
    schemeThrow("define takes exactyly 2 arguments");
  }
  symbol = getCar(argumentCons);
  if (!hasTag(symbol, TAG_SYMBOL)) {
    schemeThrow("can only define symbols");
  }
  value = getCdr(argumentCons);
  if (value == SCM_NIL || getCdr(value) != SCM_NIL) {
    schemeThrow("define takes exactyly 2 arguments");
  }
  // value = evaluate(env, getCar(value));
  value = SCM_NIL;  // TODO!
  define(*env, symbol, value);
  t_RETURN(SCM_NIL)
}

Continuation* setSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *symbol, *expression, *value;
  try {
    symbol = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    expression = getCar(argumentCons);
    if (getCdr(argumentCons) != SCM_NIL) {
      schemeThrow("set requires exactly two arguments: (set! {name} {value})");
    }
    // value = evaluate(env, expression);
    value = SCM_NIL;  // TODO!
    set(*env, symbol, value);
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("set requires exactly two arguments: (set! {name} {value})");
  }

  t_RETURN(value);
}

Continuation* quoteSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  DLOG_F(INFO, "quote arg: %s", toString(argumentCons).c_str());
  Object* quoted = (hasTag(argumentCons, TAG_CONS)) ? getCar(argumentCons) : argumentCons;
  t_RETURN(quoted);
}

Continuation* ifSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *condition, *trueExpression, *falseExpression;
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
  // TODO TRAMPOLINE
  // return (toSchemeBool(*env, condition) == SCM_TRUE) ? evaluate(env, trueExpression)
  //                                                    : evaluate(env, falseExpression);
  t_RETURN(SCM_NIL);  // TODO!
}

Continuation* beginSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *currentExpression, *lastValue{SCM_NIL};
  while (argumentCons != SCM_NIL) {
    currentExpression = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    // lastValue = evaluate(env, currentExpression);
    lastValue = SCM_NIL;
  };
  t_RETURN(lastValue);
}

Continuation* lambdaSyntax()
{
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  Object *argList, *bodyList;
  try {
    argList = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    bodyList = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    DLOG_F(INFO, "new lambda %s => %s", toString(argList).c_str(), toString(bodyList).c_str());
    if (argumentCons != SCM_NIL) {
      schemeThrow("lambda requires exactly two arguments: (lambda {argument} {body})");
    }
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("lambda requires exactly two arguments: (lambda {argument} {body})");
  }
  t_RETURN(newUserFunction(argList, bodyList, *env));
}

// BUILTIN FUNCTIONS

Continuation* addFunction()
{
  /**
   * Function that handles the addition or concatenation of multiple scm::Objects
   * @param stack: stack containing scm::Object*, which are the prepared arguments
   * @param nArgs: how many arguments the function should take
   * @return a new scm::Object* with the result of the computation
   */
  int nArgs{popArg<int>()};
  DLOG_F(WARNING, "nArgs = %d", nArgs);
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
    std::string result = std::reduce(arguments.begin(), arguments.end(), std::string{}, lambda);
    t_RETURN(newString(result));
  }

  else if (std::any_of(arguments.begin(), arguments.end(), isFloatingPoint)) {
    auto lambda = [](double a, Object* b) {
      if (hasTag(b, TAG_FLOAT)) {
        return getFloatValue(b) + a;
      }
      else {
        return static_cast<double>(getIntValue(b) + a);
      }
    };
    double result = std::reduce(arguments.begin(), arguments.end(), double(0.0), lambda);
    t_RETURN(newFloat(result));
  }

  else {
    auto lambda = [](int a, Object* b) { return getIntValue(b) + a; };
    double result = std::reduce(arguments.begin(), arguments.end(), 0, lambda);
    t_RETURN(newInteger(result));
  }
}

Continuation* subFunction()
{
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
    doubleSubtrahend = std::reduce(subtrahends.begin(), subtrahends.end(), double(0.0), lambda);
    t_RETURN(newFloat(minuend - doubleSubtrahend));
  }
  else {
    auto lambda = [](int a, Object* b) { return a + getIntValue(b); };
    intSubtrahend = std::reduce(subtrahends.begin(), subtrahends.end(), int(0), lambda);
    t_RETURN(newInteger(static_cast<int>(minuend) - intSubtrahend));
  }
}

Continuation* multFunction()
{
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
    t_RETURN(newFloat(std::reduce(arguments.begin(), arguments.end(), double(1), lambda)));
  }
  else {
    auto lambda = [](int a, Object* b) { return a * getIntValue(b); };
    t_RETURN(newInteger(std::reduce(arguments.begin(), arguments.end(), int{1}, lambda)));
  }
}

// TODO: throw out, implement in scheme
Continuation* divFunction()
{
  int nArgs{popArg<int>()};
  // Object* divisor{multFunction(stack, nArgs - 1)};
  // Object* dividend{popArg<Object*>()};

  // if (isFloatingPoint(dividend) && isFloatingPoint(divisor)) {
  //   t_RETURN(newFloat(getFloatValue(dividend) / getFloatValue(divisor)));
  // }
  // else if (isFloatingPoint(dividend)) {
  //   t_RETURN(newFloat(getFloatValue(dividend) / static_cast<double>(getIntValue(divisor))));
  // }
  // else if (isFloatingPoint(divisor)) {
  //   t_RETURN(newFloat(static_cast<double>(getIntValue(dividend)) / getFloatValue(divisor)));
  // }
  // else {
  //   t_RETURN(newFloat(static_cast<double>(getIntValue(dividend)) /
  //                     static_cast<double>(getIntValue(divisor))));
  // }
  t_RETURN(NULL);
}

Continuation* modFunction()
{
  int nArgs{popArg<int>()};
  if (nArgs != 2) {
    schemeThrow("modulo expects excactly 2 arguments");
  }
  Object* divisor{popArg<Object*>()};
  Object* dividend{popArg<Object*>()};
  if (!isNumeric(divisor) || !isNumeric(dividend)) {
    schemeThrow("modulo only works with numbers");
  }

  if (isFloatingPoint(dividend) && isFloatingPoint(divisor)) {
    t_RETURN(newFloat(std::fmod(getFloatValue(dividend), getFloatValue(divisor))));
  }
  else if (isFloatingPoint(dividend)) {
    t_RETURN(
        newFloat(std::fmod(getFloatValue(dividend), static_cast<double>(getIntValue(divisor)))));
  }
  else if (isFloatingPoint(divisor)) {
    t_RETURN(
        newFloat(std::fmod(static_cast<double>(getIntValue(dividend)), getFloatValue(divisor))));
  }
  else {
    t_RETURN(newInteger(getIntValue(dividend) % getIntValue(divisor)));
  }
}

// HIGHER ORDER FUNCTIONS

/**
 * Call a given function with the specified arguments
 * @param stack the stack from which to pop the arguments
 * @param nArgs the number of arguments
 * @return the return value of the called function
 */
Object* applyFunction()
{
  Object *function, *argumentCons;
  return SCM_NIL;
}

Continuation* eqFunction()
{
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  t_RETURN((a == b) ? SCM_TRUE : SCM_FALSE);
}

// TODO: can this be done?
// Object* compareTwoNumbers(ObjectStack& stack, int nArgs, std::function comparison)
// {
//   Object* a{popArg<Object*>()};
//   Object* b{popArg<Object*>()};
//   if (!isNumeric(a) || !isNumeric(b)) {
//     schemeThrow("= only works with numbers");
//   }
//   if (isFloatingPoint(a) && isFloatingPoint(b)) {
//     return (comparison(getFloatValue(a), getFloatValue(b))) ? SCM_TRUE : SCM_FALSE;
//   }
//   else if (isFloatingPoint(a)) {
//     return (comparison(getFloatValue(a), getIntValue(b))) ? SCM_TRUE : SCM_FALSE;
//   }
//   else if (isFloatingPoint(b)) {
//     return (comparison(getIntValue(a), getFloatValue(b))) ? SCM_TRUE : SCM_FALSE;
//   }
//   else {
//     return (comparison(getIntValue(a), getIntValue(b))) ? SCM_TRUE : SCM_FALSE;
//   }
// }

Continuation* equalNumberFunction()
{
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

//  TODO: implment this in scheme!
// Object* equalFunction(ObjectStack& stack, int nArgs)
// {
//   Object* b{popArg<Object*>()};
//   Object* a{popArg<Object*>()};
//   if (isNumeric(a) && isNumeric(b)) {
//     push(stack, {a, b});
//     return equalNumberFunction(stack, nArgs);
//   }
//   else if (getTag(a) != getTag(b)) {
//     return SCM_FALSE;
//   }
//   else if (isString(a) && isString(b)) {
//     return (getStringValue(a) == getStringValue(b)) ? SCM_TRUE : SCM_FALSE;
//   }
//   else if (hasTag(a, TAG_CONS) && hasTag(b, TAG_CONS)) {
//     schemeThrow("cons comparison not implemented yet!");
//   }
//   else {
//     schemeThrow("cannot compare objects " + toString(a) + " and " + toString(b));
//   }
// }

Continuation* greaterThanFunction()
{
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

Continuation* lesserThanFunction()
{
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

Continuation* consFunction()
{
  int nArgs{popArg<int>()};
  Object* cdr{popArg<Object*>()};
  Object* car{popArg<Object*>()};
  t_RETURN(newCons(car, cdr));
}

Continuation* carFunction()
{
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get car value from non-cons object");
  }
  t_RETURN(getCar(cons));
}

Continuation* cdrFunction()
{
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get cdr value from non-cons object");
  }
  t_RETURN(getCdr(cons));
}

Continuation* listFunction()
{
  int nArgs{popArg<int>()};
  Object* rest;
  while (nArgs--) {
    Object* currentArgument{popArg<Object*>()};
    rest = newCons(currentArgument, rest);
  }
  t_RETURN(rest);
}

Continuation* displayFunction()
{
  int nArgs{popArg<int>()};
  ObjectVec arguments{popArgs<Object*>(nArgs)};
  for (auto argument{arguments.rbegin()}; argument != arguments.rend(); argument++) {
    std::cout << toString(*argument) << " ";
  }
  std::cout << '\n';
  t_RETURN(SCM_VOID);
}

Continuation* functionBodyFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function body is not a lambda");
  }
  t_RETURN(getUserFunctionBodyList(obj));
}

Continuation* functionArglistFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function arglist is not a lambda");
  }
  t_RETURN(getUserFunctionArgList(obj));
}

Continuation* isStringFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isString(obj)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isNumberFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isNumeric(obj)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isConsFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_CONS)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isBuiltinFunctionFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_BUILTIN)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isUserFunctionFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_USER)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isBoolFunction()
{
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isOneOf(obj, {TAG_TRUE, TAG_FALSE})) ? SCM_TRUE : SCM_FALSE);
}

}  // namespace trampoline
}  // namespace scm