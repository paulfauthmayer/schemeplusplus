#pragma once
#include <iostream>
#include "environment.hpp"
#include "scheme.hpp"

namespace scm {
namespace trampoline {

Object* pop(ObjectStack& stack);
ObjectVec popN(ObjectStack& stack, int n);
inline void push(ObjectStack& stack, Object* obj);
void push(ObjectStack& stack, ObjectVec objects);
Continuation* evaluate();
Object* evaluateExpression(Environment& env, Object* obj);

}  // namespace trampoline
}  // namespace scm
