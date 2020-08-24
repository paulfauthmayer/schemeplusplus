#include "environment.hpp"
#include <exception>
#include <iostream>
#include <loguru.hpp>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {
Object* getBinding(Environment& env, Object* key)
{
  Environment* currentEnv = &env;
  while (currentEnv != NULL) {
    auto lambda = [key](Object* binding) {
      return (getStringValue(getCar(binding)) == getStringValue(key));
    };
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

Object* getVariable(Environment& env, Object* key)
{
  Object* binding{getBinding(env, key)};
  return (binding != NULL) ? getCdr(binding) : NULL;
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

}  // namespace scm