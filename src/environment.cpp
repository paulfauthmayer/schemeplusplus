#include "environment.hpp"
#include <iostream>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

Object* getVariable(Environment& env, Object* key)
{
  auto found = std::find_if(env.bindings.begin(), env.bindings.end(), [key](Object* binding) {
    return (getCar(binding) == getCar(key));
  });
  return (found != env.bindings.end()) ? *found : NULL;
}

void define(Environment& env, Object* key, Object* value)
{
  auto currentDefinition = getVariable(env, key);
  if (currentDefinition == NULL)
    env.bindings.push_back(newCons(key, value));
  else
    std::get<ConsValue>(currentDefinition->value).cdr = value;
  return;
}

}  // namespace scm