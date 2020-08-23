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
  auto lambda = [key](Object* binding) {
    return (getStringValue(getCar(binding)) == getStringValue(key));
  };
  auto found = std::find_if(env.bindings.begin(), env.bindings.end(), lambda);

  return (found != env.bindings.end()) ? *found : NULL;
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
    std::cout << getTag(currentDefinition) << "\n";
    std::get<ConsValue>(currentDefinition->value).cdr = value;
  }
}

}  // namespace scm