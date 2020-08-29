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
  return SCM_FALSE;
}

// BUILTIN SYNTAX

// forward declaration of continuation parts for syntax functions
static Continuation* beginSyntax_Part1();
static Continuation* defineSyntax_Part1();
static Continuation* ifSyntax_Part1();
static Continuation* setSyntax_Part1();

Continuation* defineSyntax()
{
  // Environment& env, scm::Object* arguments
  /**
   * Define a new variable in a given environment
   * @param env: the environment in which to define the variable
   * @param argumentCons: the arguments of the operation as a cons object
   * @return a scm::Object with the result of the operation
   */
  DLOG_F(ERROR, "defineSyntax");
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

  // push arguments required for next part
  pushArgs({env, symbol});

  // call evaluate then continue with next part
  return tCall(cont(evaluate), cont(defineSyntax_Part1), {env, getCar(value)});
}

static Continuation* defineSyntax_Part1()
{
  DLOG_F(ERROR, "defineSyntax Part1");
  Environment* env{popArg<Environment*>()};
  Object* symbol{popArg<Object*>()};
  Object* value{lastReturnValue};

  define(*env, symbol, value);
  t_RETURN(SCM_VOID)
}

Continuation* setSyntax()
{
  DLOG_F(ERROR, "setSyntax");
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

static Continuation* setSyntax_Part1()
{
  DLOG_F(ERROR, "setSyntax_Part1");
  Environment* env{popArg<Environment*>()};
  Object* symbol{popArg<Object*>()};
  Object* value{lastReturnValue};

  set(*env, symbol, value);
  t_RETURN(value);
}

Continuation* quoteSyntax()
{
  DLOG_F(ERROR, "quoteSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  DLOG_F(INFO, "quote arg: %s", toString(argumentCons).c_str());
  Object* quoted = (hasTag(argumentCons, TAG_CONS)) ? getCar(argumentCons) : argumentCons;
  t_RETURN(quoted);
}

Continuation* ifSyntax()
{
  DLOG_F(ERROR, "ifSyntax");
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
  // push arguments required for next part
  pushArgs({env, trueExpression, falseExpression});

  // call evaluate then continue with next part
  return tCall(cont(evaluate), cont(ifSyntax_Part1), {env, condition});
}
static Continuation* ifSyntax_Part1()
{
  DLOG_F(ERROR, "ifSyntax Part1");
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
      schemeThrow("evaluation not yet implemented for " + toString(evaluatedCondition));
      break;
    }
  }
  Object* expression{(conditionAsBool == SCM_TRUE) ? trueExpression : falseExpression};
  DLOG_F(WARNING, "picked expression %s", toString(expression).c_str());
  return tCall(cont(evaluate), {env, expression});
}

Continuation* beginSyntax()
{
  DLOG_F(ERROR, "beginSyntax");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};
  if (argumentCons == SCM_NIL) {
    t_RETURN(SCM_VOID);
  }
  else {
    return tCall(cont(beginSyntax_Part1), {env, argumentCons});
  }
}

static Continuation* beginSyntax_Part1()
{
  DLOG_F(ERROR, "beginSyntax Part1");
  Environment* env{popArg<Environment*>()};
  Object* argumentCons{popArg<Object*>()};

  Object* currentExpression = getCar(argumentCons);
  argumentCons = getCdr(argumentCons);

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

Continuation* lambdaSyntax()
{
  DLOG_F(ERROR, "lambdaSyntax");
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
  DLOG_F(ERROR, "addFunction");
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
  DLOG_F(ERROR, "subFunction");
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
  DLOG_F(ERROR, "multFunction");
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
  DLOG_F(ERROR, "divFunction");
  schemeThrow("division is currently not implemented, write it yourself!");
  int nArgs{popArg<int>()};
  ObjectVec arguments{popArgs<Object*>(nArgs)};
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
  DLOG_F(ERROR, "modFunction");
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
  DLOG_F(ERROR, "eqFunction");
  int nArgs{popArg<int>()};
  Object* b{popArg<Object*>()};
  Object* a{popArg<Object*>()};
  t_RETURN((a == b) ? SCM_TRUE : SCM_FALSE);
}

Continuation* equalStringFunction()
{
  DLOG_F(ERROR, "equalStringFunction");
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

Continuation* equalNumberFunction()
{
  DLOG_F(ERROR, "equalNumberFunction");
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
Continuation* equalFunction()
{
  int nArgs{popArg<int>()};
  ObjectVec arguments{popArgs<Object*>(nArgs)};
  schemeThrow("equal? is currently not implemented, write it yourself!");
  // Object* b{popArg<Object*>()};
  // Object* a{popArg<Object*>()};
  // if (isNumeric(a) && isNumeric(b)) {
  //   push(stack, {a, b});
  //   return equalNumberFunction(stack, nArgs);
  // }
  // else if (getTag(a) != getTag(b)) {
  //   return SCM_FALSE;
  // }
  // else if (isString(a) && isString(b)) {
  //   return (getStringValue(a) == getStringValue(b)) ? SCM_TRUE : SCM_FALSE;
  // }
  // else if (hasTag(a, TAG_CONS) && hasTag(b, TAG_CONS)) {
  //   schemeThrow("cons comparison not implemented yet!");
  // }
  // else {
  //   schemeThrow("cannot compare objects " + toString(a) + " and " + toString(b));
  // }
}

Continuation* greaterThanFunction()
{
  DLOG_F(ERROR, "greaterThanFunction");
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
  DLOG_F(ERROR, "lesserThanFunction");
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
  DLOG_F(ERROR, "consFunction");
  int nArgs{popArg<int>()};
  Object* cdr{popArg<Object*>()};
  Object* car{popArg<Object*>()};
  t_RETURN(newCons(car, cdr));
}

Continuation* carFunction()
{
  DLOG_F(ERROR, "carFunction");
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get car value from non-cons object");
  }
  t_RETURN(getCar(cons));
}

Continuation* cdrFunction()
{
  DLOG_F(ERROR, "cdrFunction");
  int nArgs{popArg<int>()};
  Object* cons{popArg<Object*>()};
  if (!hasTag(cons, TAG_CONS)) {
    schemeThrow("trying to get cdr value from non-cons object");
  }
  t_RETURN(getCdr(cons));
}

Continuation* listFunction()
{
  DLOG_F(ERROR, "listFunction");
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
  DLOG_F(ERROR, "displayFunction");
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
  DLOG_F(ERROR, "functionBodyFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function body is not a lambda");
  }
  t_RETURN(getUserFunctionBodyList(obj));
}

Continuation* functionArglistFunction()
{
  DLOG_F(ERROR, "functionArglistFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("function arglist is not a lambda");
  }
  t_RETURN(getUserFunctionArgList(obj));
}

Continuation* isStringFunction()
{
  DLOG_F(ERROR, "isStringFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isString(obj)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isNumberFunction()
{
  DLOG_F(ERROR, "isNumberFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isNumeric(obj)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isConsFunction()
{
  DLOG_F(ERROR, "isConsFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_CONS)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isBuiltinFunctionFunction()
{
  DLOG_F(ERROR, "isBuiltinFunctionFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_BUILTIN)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isUserFunctionFunction()
{
  DLOG_F(ERROR, "isUserFunctionFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((hasTag(obj, TAG_FUNC_USER)) ? SCM_TRUE : SCM_FALSE);
}

Continuation* isBoolFunction()
{
  DLOG_F(ERROR, "isBoolFunction");
  int nArgs{popArg<int>()};
  Object* obj{popArg<Object*>()};
  t_RETURN((isOneOf(obj, {TAG_TRUE, TAG_FALSE})) ? SCM_TRUE : SCM_FALSE);
}

}  // namespace trampoline
}  // namespace scm