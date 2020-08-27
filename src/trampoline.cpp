#include "trampoline.hpp"
#include <loguru.hpp>
#include <stack>
#include <variant>
#include "environment.hpp"
#include "scheme.hpp"

namespace scm {
namespace trampoline {

std::stack<ArgumentTypeVariant> argumentStack;
std::stack<Continuation*> functionStack;
Object* lastReturnValue = SCM_NIL;

void initializeEvaluationStacks()
{
  argumentStack = std::stack<ArgumentTypeVariant>();
  functionStack = std::stack<Continuation*>();
}

inline Continuation* tReturn(Object* value)
{
  lastReturnValue = value;
  return popFunc();
}

Continuation* tCall(Continuation* nextFunc,
                    Continuation* nextPart,
                    std::vector<ArgumentTypeVariant> arguments)
{
  pushArgs(arguments);
  if (nextPart != NULL) {
    pushFunc(nextPart);
  }
  return nextFunc;
}

/**
 * Pushes the passed argument to the top of the argument stack
 * @param arg the argument to be pushed onto the stack
 */
inline void pushArg(ArgumentTypeVariant arg)
{
  argumentStack.push(arg);
}

/**
 * Pushes a vector of arguments onto the stack in reverse order
 * this allows us to get the arguments back in the same order that we've specified them
 * @param arguments the vector that containing the arguments, can be specified as {0, obj, env}
 */
void pushArgs(std::vector<ArgumentTypeVariant> arguments)
{
  std::cout << "push " << arguments.size() << "\n";
  for (auto i = arguments.rbegin(); i != arguments.rend(); i++) {
    std::cout << "stacksize: " << argumentStack.size();
    pushArg(*i);
    std::cout << " -> " << argumentStack.size() << "\n";
  }
}

Continuation* popFunc()
{
  std::cout << "funcStack: " << functionStack.size() << "\n";
  if (functionStack.size() == 0) {
    schemeThrow("could not pop from function stack!");
  }
  Continuation* nextFunc{functionStack.top()};
  functionStack.pop();
  return nextFunc;
}

void pushFunc(Continuation* nextFunc)
{
  functionStack.push(nextFunc);
}

}  // namespace trampoline
}  // namespace scm
