#include "environment.hpp"
#include <exception>
#include <iostream>
#include <loguru.hpp>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {
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

Object* getBinding(Environment& env, Object* key)
{
  std::string keyStr = getStringValue(key);
  return getBinding(env, keyStr);
}

Object* getVariable(Environment& env, std::string& key)
{
  Object* binding{getBinding(env, key)};
  return (binding != NULL) ? getCdr(binding) : NULL;
}

Object* getVariable(Environment& env, Object* key)
{
  std::string keyStr = getStringValue(key);
  return getVariable(env, keyStr);
}

void define(Environment& env, Object* key, Object* value)
{
  DLOG_F(INFO, "define %s := %s", toString(key).c_str(), toString(value).c_str());
  auto currentDefinition = getBinding(env, key);
  if (currentDefinition == NULL) {
    DLOG_F(INFO, "define new variable %s := %s", toString(key).c_str(), toString(value).c_str());
    Object* definition{newCons(key, value)};
    env.bindings.push_back(definition);
  }
  else {
    DLOG_F(INFO, "redefine variable %s := %s", toString(key).c_str(), toString(value).c_str());
    std::get<ConsValue>(currentDefinition->value).cdr = value;
  }
}

void set(Environment& env, Object* key, Object* value)
{
  Environment* currentEnvPtr = &env;
  do {
    define(*currentEnvPtr, key, value);
    currentEnvPtr = (*currentEnvPtr).parentEnv;
  } while (currentEnvPtr != NULL);
}

void printEnv(Environment& env)
{
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