#include "environment.hpp"
#include <exception>
#include <iostream>
#include <loguru.hpp>
#include <numeric>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

/**
 * Copy constructor for the Environment class
 * @param env the environment to copy
 */
Environment::Environment(const Environment& env)
{
  parentEnv = env.parentEnv;
  bindings = env.bindings;
}

/**
 * Get a binding of the given string key in the specified Environment
 * @param env the environment in which to look
 * @param key the key to look out for
 * @returns The found binding in form con(key, value) or NULL if none was found
 */
Object* getBinding(Environment& env, std::string& key)
{
  Environment* currentEnv = &env;
  while (currentEnv != NULL) {
    auto lambda = [key](Object* binding) { return (getStringValue(getCar(binding)) == key); };
    auto found = std::find_if(currentEnv->bindings.begin(), currentEnv->bindings.end(), lambda);
    if (found != currentEnv->bindings.end()) {
      return *found;
    }
    else {
      currentEnv = currentEnv->parentEnv;
    }
  }
  return NULL;
}

/**
 * Get a binding of the given symbol Object key in the specified Environment
 * @overload
 */
Object* getBinding(Environment& env, Object* key)
{
  std::string keyStr = getStringValue(key);
  return getBinding(env, keyStr);
}

/**
 * Get the value of a binding of a given string key in the specified Environment
 * @param env the environment in which to look
 * @param key the key to look out for
 * @returns The found variable
 */
Object* getVariable(Environment& env, std::string& key)
{
  Object* binding{getBinding(env, key)};
  return (binding != NULL) ? getCdr(binding) : NULL;
}

/**
 * Get the value of a binding of a given symbol Object key in the specified Environment
 * @overload
 */
Object* getVariable(Environment& env, Object* key)
{
  std::string keyStr = getStringValue(key);
  return getVariable(env, keyStr);
}

/**
 * Define a new binding in the given environment.
 * @param env the environment in which to define
 * @param key the key of the binding
 * @param value the value of the binding
 */
void define(Environment& env, Object* key, Object* value)
{
  DLOG_IF_F(
      INFO, LOG_ENVIRONMENT, "define %s := %s", toString(key).c_str(), toString(value).c_str());
  auto currentDefinition = getBinding(env, key);
  // if binding doesn't exist yet create a new one
  if (currentDefinition == NULL) {
    DLOG_IF_F(INFO,
              LOG_ENVIRONMENT,
              "define new variable %s := %s",
              toString(key).c_str(),
              toString(value).c_str());
    Object* definition{newCons(key, value)};
    env.bindings.push_back(definition);
  }
  // if binding does exist set the old value to the new value
  else {
    DLOG_IF_F(INFO,
              LOG_ENVIRONMENT,
              "redefine variable %s := %s",
              toString(key).c_str(),
              toString(value).c_str());
    std::get<ConsValue>(currentDefinition->value).cdr = value;
  }
}

/**
 * Set a new binding in the given environment and all ancestor environments.
 * @param env the environment in which to define
 * @param key the key of the binding
 * @param value the value of the binding
 */
void set(Environment& env, Object* key, Object* value)
{
  Environment* currentEnvPtr = &env;
  // define variable in every env until no parent env can be found
  while (currentEnvPtr != NULL) {
    define(*currentEnvPtr, key, value);
    currentEnvPtr = (*currentEnvPtr).parentEnv;
  };
}

/**
 * Prints all bindings of a given environment in a formatted form.
 * @param env the environment to print
 */
void printEnv(Environment& env)
{
  // get longest variable name for spacing purposes
  int longestVariableNameLength = std::reduce(
      env.bindings.begin(), env.bindings.end(), 0, [](int longestLength, Object* binding) {
        return (getStringValue(getCar(binding)).size() > longestLength)
                   ? getStringValue(getCar(binding)).size()
                   : longestLength;
      });

  std::cout << "======== SYNTAX ========\n";
  for (auto binding : env.bindings) {
    if (hasTag(getCdr(binding), TAG_SYNTAX)) {
      std::string name = getStringValue(getCar(binding));
      std::cout << toString(getCar(binding));
      for (int i{0}; i < longestVariableNameLength - name.size(); i++) {
        std::cout << ' ';
      }
      std::cout << " :=  " << toString(getCdr(binding)) << "\n";
    }
  }
  std::cout << "======== FUNCTIONS ========\n";
  for (auto binding : env.bindings) {
    if (hasTag(getCdr(binding), TAG_FUNC_BUILTIN)) {
      std::string name = getStringValue(getCar(binding));
      std::cout << toString(getCar(binding));
      for (int i{0}; i < longestVariableNameLength - name.size(); i++) {
        std::cout << ' ';
      }
      std::cout << " :=  " << toString(getCdr(binding)) << "\n";
    }
  }
  for (auto binding : env.bindings) {
    if (hasTag(getCdr(binding), TAG_FUNC_USER)) {
      std::string name = getStringValue(getCar(binding));
      std::cout << toString(getCar(binding));
      for (int i{0}; i < longestVariableNameLength - name.size(); i++) {
        std::cout << ' ';
      }
      std::cout << " :=  function args: " << toString(getUserFunctionArgList(getCdr(binding)))
                << "\n";
    }
  }
  std::cout << "======== VARIABLES ========\n";
  for (auto binding : env.bindings) {
    if (!isOneOf(getCdr(binding), {TAG_FUNC_BUILTIN, TAG_FUNC_USER, TAG_SYNTAX})) {
      std::string name = getStringValue(getCar(binding));
      std::cout << toString(getCar(binding));
      for (int i{0}; i < longestVariableNameLength - name.size(); i++) {
        std::cout << ' ';
      }
      std::cout << " := " << toString(getCdr(binding)) << "\n";
    }
  }
  std::cout << "===========================\n";
}
}  // namespace scm