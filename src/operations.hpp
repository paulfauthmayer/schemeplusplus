#include <iostream>
#include <stack>
#include "scheme.hpp"
namespace scm {

// SYNTAX

// BUILTIN FUNCTIONS
Object* addFunction(std::stack<Object*>& stack, int nArgs);
Object* subFunction(std::stack<Object*> stack, int nArgs);
Object* multFunction(std::stack<Object*> stack, int nArgs);
Object* divFunction(std::stack<Object*> stack, int nArgs);
Object* modFunction(std::stack<Object*> stack, int nArgs);
Object* eqFunction(std::stack<Object*> stack, int nArgs);
Object* equalFunction(std::stack<Object*> stack, int nArgs);
Object* equalNumberFunction(std::stack<Object*> stack, int nArgs);
Object* greaterThanFunction(std::stack<Object*> stack, int nArgs);
Object* lesserThanFunction(std::stack<Object*> stack, int nArgs);
Object* consFunction(std::stack<Object*> stack, int nArgs);
Object* carFunction(std::stack<Object*> stack, int nArgs);
Object* cdrFunction(std::stack<Object*> stack, int nArgs);
Object* listFunction(std::stack<Object*> stack, int nArgs);
Object* displayFunction(std::stack<Object*> stack, int nArgs);
Object* functionBodyFunction(std::stack<Object*> stack, int nArgs);
Object* functionArglistFunction(std::stack<Object*> stack, int nArgs);
Object* isStringFunction(std::stack<Object*> stack, int nArgs);
Object* isNumberFunction(std::stack<Object*> stack, int nArgs);
Object* isConsFunction(std::stack<Object*> stack, int nArgs);
Object* isFunctionFunction(std::stack<Object*> stack, int nArgs);
Object* isUserFunctionFunction(std::stack<Object*> stack, int nArgs);
Object* isBoolFunction(std::stack<Object*> stack, int nArgs);

// USER DEFINED FUNCTIONS

}  // namespace scm
