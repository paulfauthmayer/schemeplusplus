#pragma once
#include <vector>
#include "scheme.hpp"

namespace scm {
class Environment {
 private:
  /* data */
  std::vector<scm::Object*> bindings;
  Environment* parentEnv;

 public:
  Environment(Environment* parent = NULL) : parentEnv(parent){};
  ~Environment() = default;
  friend void define(Environment& env, Object* key, Object* value);
  friend Object* getVariable(Environment& env, Object* key);
};

void define(Environment& env, Object* key, Object* value);
Object* getVariable(Environment& env, Object* key);

}  // namespace scm
