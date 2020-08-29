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

Continuation* tCall(Continuation* nextFunc, std::vector<ArgumentTypeVariant> arguments)
{
  return tCall(nextFunc, NULL, arguments);
}

void printArg(ArgumentTypeVariant arg, std::string prefix, std::string postfix)
{
  if (std::holds_alternative<Environment*>(arg)) {
    DLOG_IF_F(INFO, LOG_STACK_TRACE, "%s env %s", prefix.c_str(), postfix.c_str());
  }
  else if (std::holds_alternative<Object*>(arg)) {
    DLOG_IF_F(INFO,
              LOG_STACK_TRACE,
              "%s object %s %s",
              prefix.c_str(),
              toString(std::get<Object*>(arg)).c_str(),
              postfix.c_str());
  }
  else if (std::holds_alternative<int>(arg)) {
    DLOG_IF_F(
        INFO, LOG_STACK_TRACE, "%s int %d %s", prefix.c_str(), std::get<int>(arg), postfix.c_str());
  }
  else if (std::holds_alternative<std::size_t>(arg)) {
    DLOG_IF_F(INFO,
              LOG_STACK_TRACE,
              "%s size_t %d %s",
              prefix.c_str(),
              static_cast<int>(std::get<std::size_t>(arg)),
              postfix.c_str());
  }
}

void printArgStack()
{
  std::stack<ArgumentTypeVariant> placeholder{};
  DLOG_IF_F(
      INFO, LOG_STACK_TRACE, "argstack - %d arguments", static_cast<int>(argumentStack.size()));
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
  if (functionStack.size() == 0) {
    schemeThrow("could not pop from function stack!");
  }
  DLOG_IF_F(INFO,
            LOG_STACK_TRACE,
            "pop function [%d->%d]",
            static_cast<int>(functionStack.size()),
            static_cast<int>(functionStack.size() - 1));
  Continuation* nextFunc{functionStack.top()};
  functionStack.pop();
  return nextFunc;
}

void pushFunc(Continuation* nextFunc)
{
  DLOG_IF_F(INFO,
            LOG_STACK_TRACE,
            "push function : %d -> %d",
            static_cast<int>(functionStack.size()),
            static_cast<int>(functionStack.size() + 1));
  functionStack.push(nextFunc);
}

}  // namespace trampoline
}  // namespace scm
