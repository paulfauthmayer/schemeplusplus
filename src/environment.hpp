#pragma once
#include <map>
#include "scheme.hpp"

namespace scm {

/**
 * Used as a container for variable definitions. All functions, syntax and user defined
 * objects are stored in an environment. They are organised in an hierarchical manner with each
 * Environment object pointing to its parent Environment. Therefore, children have access to the
 * variables defined in their parent Environment but not vice versa.
 */
class Environment {
 private:
  std::map<std::string, Object*> bindings;
  Environment* parentEnv;

 public:
  Environment(Environment* parent = NULL) : parentEnv(parent){};
  Environment(const Environment& obj);
  ~Environment() = default;
  friend void set(Environment& env, Object* key, Object* value);
  friend void define(Environment& env, std::string& key, Object* value);
  friend void printCategory(Environment& env,
                            std::function<bool(Object*)> checkFunction,
                            int maxNameLength);
  friend void printEnv(Environment& env);
  friend Object* getVariable(Environment& env, Object* key);
  friend Object* getVariable(Environment& env, std::string& key);
};

void define(Environment& env, Object* key, Object* value);
void define(Environment& env, std::string& key, Object* value);
void set(Environment& env, Object* key, Object* value);
void printEnv(Environment& env);
Object* getVariable(Environment& env, Object* key);
Object* getVariable(Environment& env, std::string& key);

}  // namespace scm
