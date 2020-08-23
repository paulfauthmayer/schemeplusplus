#include "operations.hpp"
#include <algorithm>
#include <iostream>
#include <loguru.hpp>
#include <numeric>
#include <stack>
#include <string>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

Object* pop(ObjectStack& stack)
{
  /**
   * Pops and returns the topmost element of a given ObjectStack
   * @param stack the stack from which to pop
   * @return the popped scm::Object*
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
   * @return the popped objects in a ObjectVec
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

// BUILTIN SYNTAX
Object* defineSyntax(ObjectStack& stack, int nArgs)
{
  /**
   * Define a new variable in a given environment
   * @param stack: stack containing scm::Object*, which are the prepared arguments
   * @param nArgs: how many arguments the function should take
   * @return SCM_VOID
   */
  auto arguments = popN(stack, nArgs);
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

// Object* multFunction(ObjectStack stack, int nArgs);
// Object* divFunction(ObjectStack stack, int nArgs);
// Object* modFunction(ObjectStack stack, int nArgs);
// Object* eqFunction(ObjectStack stack, int nArgs);
// Object* equalFunction(ObjectStack stack, int nArgs);
// Object* equalNumberFunction(ObjectStack stack, int nArgs);
// Object* greaterThanFunction(ObjectStack stack, int nArgs);
// Object* lesserThanFunction(ObjectStack stack, int nArgs);
// Object* consFunction(ObjectStack stack, int nArgs);
// Object* carFunction(ObjectStack stack, int nArgs);
// Object* cdrFunction(ObjectStack stack, int nArgs);
// Object* listFunction(ObjectStack stack, int nArgs);
// Object* displayFunction(ObjectStack stack, int nArgs);
// Object* functionBodyFunction(ObjectStack stack, int nArgs);
// Object* functionArglistFunction(ObjectStack stack, int nArgs);
// Object* isStringFunction(ObjectStack stack, int nArgs);
// Object* isNumberFunction(ObjectStack stack, int nArgs);
// Object* isConsFunction(ObjectStack stack, int nArgs);
// Object* isFunctionFunction(ObjectStack stack, int nArgs);
// Object* isUserFunctionFunction(ObjectStack stack, int nArgs);
// Object* isBoolFunctionFunction(ObjectStack stack, int nArgs);

}  // namespace scm