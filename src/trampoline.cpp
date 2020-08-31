#include "trampoline.hpp"
#include <loguru.hpp>
#include <stack>
#include <variant>
#include "environment.hpp"
#include "scheme.hpp"

namespace scm {
namespace trampoline {

/**
 * the stack that contains arguments that are passed
 * on to the following functions in the trampoline.
 */
std::stack<ArgumentTypeVariant> argumentStack;
/**
 * the stack that contains the following functions in the trampoline.
 */
std::stack<Continuation*> functionStack;
/**
 * a container to keep the last return value of all functions
 */
Object* lastReturnValue = SCM_NIL;

/**
 * Setup both stacks.
 */
void initializeEvaluationStacks()
{
  argumentStack = std::stack<ArgumentTypeVariant>();
  functionStack = std::stack<Continuation*>();
}

/**
 * Return the next function to run, push the function after the next one to the stack
 * and push the required arguments for the next function to the stack.
 * @param nextFunc the next function to call
 * @param nextPart the continuation of the current function
 * @param arguments a vector of arguments to be pushed to the stack
 * @return a pointer to the next function
 */
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
 * Return the next function to run and push the required arguments for the next function to the
 * stack.
 * @overload
 * @param nextFunc the next function to call
 * @param arguments a vector of arguments to be pushed to the stack
 * @returns a pointer to the next function
 */
Continuation* tCall(Continuation* nextFunc, std::vector<ArgumentTypeVariant> arguments)
{
  return tCall(nextFunc, NULL, arguments);
}

/**
 * Log an argument from the argument stack.
 * @param arg the argument to print
 * @param prefix prefix for the log message
 * @param postfix postfix for the log message
 */
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

/**
 * Log the entire argument stack.
 */
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

/**
 * Pops the next function from the function stack
 * @returns a pointer to the next function
 */
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

/**
 * Pushes a function on the function stack
 * @param nextFunc the function pointer to push on the stack
 */
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
