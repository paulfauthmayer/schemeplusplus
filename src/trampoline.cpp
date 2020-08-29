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
  DLOG_F(ERROR, "tCall expanded");
  pushArgs(arguments);
  if (nextPart != NULL) {
    pushFunc(nextPart);
  }
  DLOG_F(ERROR, "tCall expanded return");
  return nextFunc;
}

Continuation* tCall(Continuation* nextFunc, std::vector<ArgumentTypeVariant> arguments)
{
  DLOG_F(ERROR, "tCall");
  return tCall(nextFunc, NULL, arguments);
}

void printArg(ArgumentTypeVariant arg, std::string action = "")
{
  if (std::holds_alternative<Environment*>(arg)) {
    DLOG_F(WARNING, "%s arg: env stackSize before: %d", action.c_str(), argumentStack.size());
  }
  else if (std::holds_alternative<Object*>(arg)) {
    DLOG_F(WARNING,
           "%s object %s -> stackSize before: %d",
           action.c_str(),
           toString(std::get<Object*>(arg)).c_str(),
           argumentStack.size());
  }
  else if (std::holds_alternative<int>(arg)) {
    DLOG_F(WARNING,
           "%s int %d -> stackSize before: %d",
           action.c_str(),
           std::get<int>(arg),
           argumentStack.size());
  }
  else if (std::holds_alternative<std::size_t>(arg)) {
    DLOG_F(WARNING,
           "%s size_t %d -> stackSize before: %d",
           action.c_str(),
           std::get<std::size_t>(arg),
           argumentStack.size());
  }
}

void printArgStack()
{
  std::stack<ArgumentTypeVariant> placeholder{};
  DLOG_F(WARNING, "argstack - %d arguments", argumentStack.size());
  while (argumentStack.size()) {
    printArg(argumentStack.top());
    placeholder.push(argumentStack.top());
    argumentStack.pop();
  }
  while (placeholder.size()) {
    argumentStack.push(placeholder.top());
    placeholder.pop();
  }
}

/**
 * Pushes the passed argument to the top of the argument stack
 * @param arg the argument to be pushed onto the stack
 */
void pushArg(ArgumentTypeVariant arg)
{
  // printArg(arg, "pushing");
  argumentStack.push(arg);
}

/**
 * Pushes a vector of arguments onto the stack in reverse order
 * this allows us to get the arguments back in the same order that we've specified them
 * @param arguments the vector that containing the arguments, can be specified as {0, obj, env}
 */
void pushArgs(std::vector<ArgumentTypeVariant> arguments)
{
  for (auto i = arguments.rbegin(); i != arguments.rend(); i++) {
    pushArg(*i);
  }
}

Continuation* popFunc()
{
  DLOG_F(WARNING, "funcStack: %d", functionStack.size());
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
