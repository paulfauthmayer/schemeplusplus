#pragma once
#include <loguru.hpp>
#include <stack>
#include <variant>
#include "environment.hpp"
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {
namespace trampoline {

// define a stack that can hold multiple different types of values objects!
// the values contained are the arguments used within our functions, as it's impossible
// to pass arguments per function call with our implementation of trampoline
using ArgumentTypeVariant = std::variant<Object*, Environment*, Continuation*, std::size_t, int>;
extern std::stack<ArgumentTypeVariant> argumentStack;

// this is the stack on which we push the next functions to call
extern FunctionStack functionStack;

// we use this variable as a container for the return value of the most recently
// finished function
extern Object* lastReturnValue;

Continuation* tCall(Continuation* nextFunc,
                    Continuation* nextPart = NULL,
                    std::vector<ArgumentTypeVariant> arguments = {});
Continuation* tCall(Continuation* nextFunc, std::vector<ArgumentTypeVariant> arguments = {});
Continuation* tReturn(Object* value);
void initializeEvaluationStacks();
template <typename T>
T popArg();
template <typename T>
std::vector<T> popArgs(int n);
void pushArg(ArgumentTypeVariant arg);
void pushArgs(std::vector<ArgumentTypeVariant> arguments);
Continuation* popFunc();
void pushFunc(Continuation* nextFunc);
void printArg(ArgumentTypeVariant arg, std::string action);
void printArgStack();

/**
 * Pops and returns the topmost element of the argument stack. Implemented because
 * the member `pop` function of std::stack doesn't return the popped value
 * @return the popped argument, can be of any type in ArgumentType Variant
 */
template <typename T>
T popArg()
{
  if (argumentStack.empty()) {
    schemeThrow("trying to pop argument from empty stack");
  }
  printArg(argumentStack.top(), "popping into " + std::string(typeid(T).name()) + " :");
  T arg{std::get<T>(argumentStack.top())};
  argumentStack.pop();
  return arg;
}

/**
 * Pops and returns the topmost N elements of the argument stack
 * @param n the amount of values popped
 * @return the popped objects in a vector of the specified type
 */
template <typename T>
std::vector<T> popArgs(int n)
{
  DLOG_F(INFO, "popping %d values from stack", n);
  if (argumentStack.size() < n) {
    printArgStack();
    schemeThrow("stack doesn't contain " + std::to_string(n) + " arguments!");
  }
  std::vector<T> values;
  try {
    for (int i{0}; i < n; i++) {
      values.push_back(popArg<T>());
    }
  }
  catch (std::bad_variant_access& e) {
    schemeThrow("argument on argument stack isn't of the chosen type");
  }
  return values;
}
}  // namespace trampoline
}  // namespace scm
