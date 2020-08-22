#include "environment.hpp"
#include <exception>
#include <iostream>
#include <loguru.hpp>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

Object* getVariable(Environment& env, Object* key)
{
  auto lambda = [key](Object* binding) {
    return (getStringValue(getCar(binding)) == getStringValue(key));
  };
  auto found = std::find_if(env.bindings.begin(), env.bindings.end(), lambda);

  return (found != env.bindings.end()) ? getCdr(*found) : NULL;
}

void define(Environment& env, Object* key, Object* value)
{
  auto currentDefinition = getVariable(env, key);
  if (currentDefinition == NULL) {
    Object* definition{newCons(key, value)};
    env.bindings.push_back(definition);
    DLOG_F(INFO, "define %s := %s", toString(key).c_str(), toString(value).c_str());
  }
  else {
    std::get<ConsValue>(currentDefinition->value).cdr = value;
    std::cout << "redefine " << toString(key) << '\n';
  }
}

}  // namespace scm