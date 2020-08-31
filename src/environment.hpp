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
  friend void set(Environment& env, Object* key, Object* value);
  friend void define(Environment& env, Object* key, Object* value);
  friend void printEnv(Environment& env);
  friend Object* getBinding(Environment& env, Object* key);
  friend Object* getBinding(Environment& env, std::string& key);
  friend Object* getVariable(Environment& env, Object* key);
  friend Object* getVariable(Environment& env, std::string& key);
};

void define(Environment& env, Object* key, Object* value);
void set(Environment& env, Object* key, Object* value);
void printEnv(Environment& env);
Object* getVariable(Environment& env, Object* key);
Object* getVariable(Environment& env, std::string& key);

}  // namespace scm
