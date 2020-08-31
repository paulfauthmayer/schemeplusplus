#pragma once
#include <iostream>
#include "environment.hpp"
#include "scheme.hpp"

namespace scm {

Object* pop(ObjectStack& stack);
ObjectVec popN(ObjectStack& stack, int n);
inline void push(ObjectStack& stack, Object* obj);
void push(ObjectStack& stack, ObjectVec objects);
Object* evaluate(Environment& env, Object* obj);

}  // namespace scm
