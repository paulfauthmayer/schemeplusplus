#include "environment.hpp"
#include <exception>
#include <iostream>
#include <loguru.hpp>
#include <map>
#include <numeric>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

/**
 * Helper function to get the keys of any std::map as a vector
 * @tparam TYPE_KEY the type of the keys
 * @tparam TYPE_VALUE the type of the values
 * @param inputMap the map from which to get the keys
 * @returns a vector<TYPE_KEY> of all keys of inputMap
 */
template <typename TYPE_KEY, typename TYPE_VALUE>
std::vector<TYPE_KEY> getKeys(std::map<TYPE_KEY, TYPE_VALUE> const& inputMap)
{
  std::vector<TYPE_KEY> keys;
  for (auto const& element : inputMap) {
    keys.push_back(element.first);
  }
  return keys;
}

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
 * Get the value of a binding of a given string key in the specified Environment
 * @param env the environment in which to look
 * @param key the key to look out for
 * @returns The found variable
 */
Object* getVariable(Environment& env, std::string& key)
{
  Environment* currentEnvPtr = &env;
  while (currentEnvPtr != NULL) {
    std::map<std::string, Object*>::iterator found = currentEnvPtr->bindings.find(key);
    if (found != currentEnvPtr->bindings.end()) {
      return currentEnvPtr->bindings.at(key);
    }
    else {
      currentEnvPtr = currentEnvPtr->parentEnv;
    }
  }
  return NULL;
}

/**
 * Get the value of a binding of a given symbol Object key in the specified Environment
 * @overload
 */
Object* getVariable(Environment& env, Object* key)
{
  if (!hasTag(key, TAG_SYMBOL)) {
    schemeThrow("values can only be bound to symbols");
  }
  std::string keyStr{getStringValue(key)};
  return getVariable(env, keyStr);
}

/**
 * Define a new binding in the given environment.
 * @param env the environment in which to define
 * @param key the key of the binding
 * @param value the value of the binding
 */
void define(Environment& env, std::string& key, Object* value)
{
  DLOG_IF_F(INFO, LOG_ENVIRONMENT, "define %s := %s", key.c_str(), toString(value).c_str());
  env.bindings[key] = value;
}

/**
 * Define a new binding in the given environment, takes an Object* as key.
 * @overload
 */
void define(Environment& env, Object* key, Object* value)
{
  if (!hasTag(key, TAG_SYMBOL)) {
    schemeThrow("values can only be bound to symbols");
  }
  std::string keyStr{getStringValue(key)};
  define(env, keyStr, value);
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
 * Print all bindings of an environment that fulfill a certain condition.
 * @param env the environment from which to get the bindings
 * @param checkFunction a function that's called on each element to determine whether it should be
 * printed or not
 * @param maxNameLength the longest variable name in the environment, for spacing purposes
 */
void printCategory(Environment& env, std::function<bool(Object*)> checkFunction, int maxNameLength)
{
  for (auto& binding : env.bindings) {
    if (checkFunction(binding.second)) {
      std::string name = binding.first;
      std::cout << name;
      for (int i{0}; i < maxNameLength - name.size(); i++) {
        std::cout << ' ';
      }
      std::cout << " :=  ";
      if (hasTag(binding.second, TAG_FUNC_USER)) {
        std::cout << toString(getUserFunctionArgList(binding.second));
      }
      else if (isOneOf(binding.second, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
        std::cout << getBuiltinFuncHelpText(binding.second)
                         .substr(0, getBuiltinFuncHelpText(binding.second).find('\n'));
      }
      else {
        std::cout << toString(binding.second);
      }
      std::cout << "\n";
    }
  }
}

/**
 * Prints all bindings of a given environment in a formatted form.
 * @param env the environment to print
 */
void printEnv(Environment& env)
{
  // get longest variable name for spacing purposes
  std::vector<std::string> keys{getKeys(env.bindings)};
  int longestVariableNameLength =
      std::accumulate(keys.begin(), keys.end(), 0, [](int longestLength, std::string& binding) {
        return (binding.size() > longestLength) ? binding.size() : longestLength;
      });

  std::cout << "======== SYNTAX ========\n";
  std::function<bool(Object*)> lambda = [](Object* obj) { return hasTag(obj, TAG_SYNTAX); };
  printCategory(env, lambda, longestVariableNameLength);
  std::cout << "======== FUNCTIONS ========\n";
  lambda = [](Object* obj) { return isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_FUNC_USER}); };
  printCategory(env, lambda, longestVariableNameLength);
  std::cout << "======== VARIABLES ========\n";
  lambda = [](Object* obj) { return !isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_FUNC_USER, TAG_SYNTAX}); };
  printCategory(env, lambda, longestVariableNameLength);
  std::cout << "===========================\n";
}
}  // namespace scm