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

namespace scm {

// HELPER FUNCTIONS
// TODO: maybe move this out, don't require env but require evaluated Object?
Object* toSchemeBool(Environment& env, Object* obj)
{
  Object* evaluatedObject{evaluate(env, obj)};
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
      schemeThrow("evaluation not yet implemented for " + scm::toString(obj));
      break;
    }
  }
  return SCM_FALSE;
}

// BUILTIN SYNTAX
Object* defineSyntax(Environment& env, scm::Object* arguments)
{
  /**
   * Define a new variable in a given environment
   * @param env: the environment in which to define the variable
   * @param arguments: the arguments of the operation as a cons object
   * @return a scm::Object with the result of the operation
   */
  Object *symbol, *value;
  if (arguments == SCM_NIL) {
    schemeThrow("define takes exactyly 2 arguments");
  }
  symbol = getCar(arguments);
  if (!hasTag(symbol, TAG_SYMBOL)) {
    schemeThrow("can only define symbols");
  }
  value = getCdr(arguments);
  if (value == SCM_NIL || getCdr(value) != SCM_NIL) {
    schemeThrow("define takes exactyly 2 arguments");
  }
  value = evaluate(env, getCar(value));
  define(env, symbol, value);
  return SCM_VOID;
}

Object* setSyntax(Environment& env, Object* argumentCons)
{
  Object *symbol, *expression, *value;
  try {
    symbol = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    expression = getCar(argumentCons);
    if (getCdr(argumentCons) != SCM_NIL) {
      schemeThrow("set requires exactly two arguments: (set! {name} {value})");
    }
    value = evaluate(env, expression);
    set(env, symbol, value);
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("set requires exactly two arguments: (set! {name} {value})");
  }

  return value;
}

Object* quoteSyntax(Object* argumentCons)
{
  return argumentCons;
}

Object* ifSyntax(Environment& env, Object* argumentCons)
{
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
  return (toSchemeBool(env, condition) == SCM_TRUE) ? evaluate(env, trueExpression)
                                                    : evaluate(env, falseExpression);
}

Object* beginSyntax(Environment& env, Object* argumentCons)
{
  Object *currentExpression, *lastValue{SCM_NIL};
  while (argumentCons != SCM_NIL) {
    currentExpression = getCar(argumentCons);
    argumentCons = getCdr(argumentCons);
    lastValue = evaluate(env, currentExpression);
  };
  return lastValue;
}

Object* lambdaSyntax(Environment& env, Object* argumentCons)
{
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
  return newUserFunction(argList, bodyList, env);
}

// BUILTIN FUNCTIONS

Object* addFunction(ObjectStack& stack, int nArgs)
{
  /**
   * Function that handles the addition or concatenation of multiple scm::Objects
   * @param stack: stack containing scm::Object*, which are the prepared arguments
   * @param nArgs: how many arguments the function should take
   * @return a new scm::Object* with the result of the computation
   */

  // get all arguments necessary and check for type validity
  if (nArgs <= 0) {
    schemeThrow("expected at least 1 argument");
  }
  auto arguments = popN(stack, nArgs);
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
    return newString(result);
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
    return newFloat(result);
  }

  else {
    auto lambda = [](int a, Object* b) { return getIntValue(b) + a; };
    double result = std::reduce(arguments.begin(), arguments.end(), 0, lambda);
    return newInteger(result);
  }
}

Object* subFunction(ObjectStack& stack, int nArgs)
{
  // TODO: handle floats as int.int
  auto subtrahends = popN(stack, nArgs - 1);
  int intSubtrahend{};
  double doubleSubtrahend;
  // TODO: this is a really ugly hack, fix this! it works, but gives the option for floating point
  // errors
  Object* minuendObj = pop(stack);
  double minuend = hasTag(minuendObj, TAG_FLOAT) ? getFloatValue(minuendObj)
                                                 : static_cast<double>(getIntValue(minuendObj));

  if (subtrahends.size() == 0) {
    if (hasTag(minuendObj, TAG_FLOAT))
      return newFloat(-getFloatValue(minuendObj));
    return newInteger(-getIntValue(minuendObj));
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
    return newFloat(minuend - doubleSubtrahend);
  }
  else {
    auto lambda = [](int a, Object* b) { return a + getIntValue(b); };
    intSubtrahend = std::reduce(subtrahends.begin(), subtrahends.end(), int(0), lambda);
    return newInteger(static_cast<int>(minuend) - intSubtrahend);
  }
}

Object* multFunction(ObjectStack& stack, int nArgs)
{
  ObjectVec arguments{popN(stack, nArgs)};
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
    return newFloat(std::reduce(arguments.begin(), arguments.end(), double(1), lambda));
  }
  else {
    auto lambda = [](int a, Object* b) { return a * getIntValue(b); };
    return newInteger(std::reduce(arguments.begin(), arguments.end(), int{1}, lambda));
  }
}

Object* divFunction(ObjectStack& stack, int nArgs)
{
  Object* divisor{multFunction(stack, nArgs - 1)};
  Object* dividend{pop(stack)};

  if (isFloatingPoint(dividend) && isFloatingPoint(divisor)) {
    return newFloat(getFloatValue(dividend) / getFloatValue(divisor));
  }
  else if (isFloatingPoint(dividend)) {
    return newFloat(getFloatValue(dividend) / static_cast<double>(getIntValue(divisor)));
  }
  else if (isFloatingPoint(divisor)) {
    return newFloat(static_cast<double>(getIntValue(dividend)) / getFloatValue(divisor));
  }
  else {
    return newFloat(static_cast<double>(getIntValue(dividend)) /
                    static_cast<double>(getIntValue(divisor)));
  }
}
Object* modFunction(ObjectStack& stack, int nArgs)
{
  if (nArgs != 2) {
    schemeThrow("modulo expects excactly 2 arguments");
  }
  Object* divisor{pop(stack)};
  Object* dividend{pop(stack)};
  if (!isNumeric(divisor) || !isNumeric(dividend)) {
    schemeThrow("modulo only works with numbers");
  }

  if (isFloatingPoint(dividend) && isFloatingPoint(divisor)) {
    return newFloat(std::fmod(getFloatValue(dividend), getFloatValue(divisor)));
  }
  else if (isFloatingPoint(dividend)) {
    return newFloat(std::fmod(getFloatValue(dividend), static_cast<double>(getIntValue(divisor))));
  }
  else if (isFloatingPoint(divisor)) {
    return newFloat(std::fmod(static_cast<double>(getIntValue(dividend)), getFloatValue(divisor)));
  }
  else {
    return newInteger(getIntValue(dividend) % getIntValue(divisor));
  }
}

Object* eqFunction(ObjectStack& stack, int nArgs)
{
  Object* b{pop(stack)};
  Object* a{pop(stack)};
  return (a == b) ? SCM_TRUE : SCM_FALSE;
}

// TODO: can this be done?
// Object* compareTwoNumbers(ObjectStack& stack, int nArgs, std::function comparison)
// {
//   Object* a{pop(stack)};
//   Object* b{pop(stack)};
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

Object* equalNumberFunction(ObjectStack& stack, int nArgs)
{
  Object* b{pop(stack)};
  Object* a{pop(stack)};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    return (getFloatValue(a) == getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(a)) {
    return (getFloatValue(a) == getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(b)) {
    return (getIntValue(a) == getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else {
    return (getIntValue(a) == getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
}

Object* equalFunction(ObjectStack& stack, int nArgs)
{
  Object* b{pop(stack)};
  Object* a{pop(stack)};
  if (isNumeric(a) && isNumeric(b)) {
    push(stack, {a, b});
    return equalNumberFunction(stack, nArgs);
  }
  else if (getTag(a) != getTag(b)) {
    return SCM_FALSE;
  }
  else if (isString(a) && isString(b)) {
    return (getStringValue(a) == getStringValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (hasTag(a, TAG_CONS) && hasTag(b, TAG_CONS)) {
    schemeThrow("cons comparison not implemented yet!");
  }
  else {
    schemeThrow("cannot compare objects " + toString(a) + " and " + toString(b));
  }
}

Object* greaterThanFunction(ObjectStack& stack, int nArgs)
{
  Object* b{pop(stack)};
  Object* a{pop(stack)};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    return (getFloatValue(a) > getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(a)) {
    return (getFloatValue(a) > getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(b)) {
    return (getIntValue(a) > getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else {
    return (getIntValue(a) > getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
}

Object* lesserThanFunction(ObjectStack& stack, int nArgs)
{
  Object* b{pop(stack)};
  Object* a{pop(stack)};
  if (!isNumeric(a) || !isNumeric(b)) {
    schemeThrow("= only works with numbers");
  }
  if (isFloatingPoint(a) && isFloatingPoint(b)) {
    return (getFloatValue(a) < getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(a)) {
    return (getFloatValue(a) < getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else if (isFloatingPoint(b)) {
    return (getIntValue(a) < getFloatValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
  else {
    return (getIntValue(a) < getIntValue(b)) ? SCM_TRUE : SCM_FALSE;
  }
}

Object* consFunction(ObjectStack& stack, int nArgs)
{
  Object* cdr{pop(stack)};
  Object* car{pop(stack)};
  return newCons(car, cdr);
}

Object* carFunction(ObjectStack& stack, int nArgs)
{
  Object* cons{pop(stack)};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get car value from non-cons object");
  }
  return getCar(cons);
}

Object* cdrFunction(ObjectStack& stack, int nArgs)
{
  Object* cons{pop(stack)};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get cdr value from non-cons object");
  }
  return getCdr(cons);
}

Object* listFunction(ObjectStack& stack, int nArgs)
{
  Object* rest;
  while (nArgs--) {
    Object* currentArgument{pop(stack)};
    rest = newCons(currentArgument, rest);
  }
  return rest;
}

Object* displayFunction(ObjectStack& stack, int nArgs)
{
  ObjectVec arguments{popN(stack, nArgs)};
  for (auto argument{arguments.rbegin()}; argument != arguments.rend(); argument++) {
    std::cout << toString(*argument) << " ";
  }
  std::cout << '\n';
  return SCM_VOID;
}

Object* functionBodyFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function body is not a lambda");
  }
  return getUserFunctionBodyList(obj);
}

Object* functionArglistFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function arglist is not a lambda");
  }
  return getUserFunctionArgList(obj);
}

Object* isStringFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (isString(obj)) ? SCM_TRUE : SCM_FALSE;
}

Object* isNumberFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (isNumeric(obj)) ? SCM_TRUE : SCM_FALSE;
}

Object* isConsFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (hasTag(obj, TAG_CONS)) ? SCM_TRUE : SCM_FALSE;
}

Object* isBuiltinFunctionFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (hasTag(obj, TAG_FUNC_BUILTIN)) ? SCM_TRUE : SCM_FALSE;
}

Object* isUserFunctionFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (hasTag(obj, TAG_FUNC_USER)) ? SCM_TRUE : SCM_FALSE;
}

Object* isBoolFunction(ObjectStack& stack, int nArgs)
{
  Object* obj{pop(stack)};
  return (isOneOf(obj, {TAG_TRUE, TAG_FALSE})) ? SCM_TRUE : SCM_FALSE;
}
}  // namespace scm