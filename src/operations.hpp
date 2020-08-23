#include <iostream>
#include <stack>
#include "scheme.hpp"
namespace scm {

// SYNTAX

// BUILTIN FUNCTIONS
Object* addFunction(ObjectStack& stack, int nArgs);
Object* subFunction(ObjectStack& stack, int nArgs);
Object* multFunction(ObjectStack& stack, int nArgs);
Object* divFunction(ObjectStack& stack, int nArgs);
Object* modFunction(ObjectStack& stack, int nArgs);
Object* eqFunction(ObjectStack& stack, int nArgs);
Object* equalFunction(ObjectStack& stack, int nArgs);
Object* equalNumberFunction(ObjectStack& stack, int nArgs);
Object* greaterThanFunction(ObjectStack& stack, int nArgs);
Object* lesserThanFunction(ObjectStack& stack, int nArgs);
Object* consFunction(ObjectStack& stack, int nArgs);
Object* carFunction(ObjectStack& stack, int nArgs);
Object* cdrFunction(ObjectStack& stack, int nArgs);
Object* listFunction(ObjectStack& stack, int nArgs);
Object* displayFunction(ObjectStack& stack, int nArgs);
Object* functionBodyFunction(ObjectStack& stack, int nArgs);
Object* functionArglistFunction(ObjectStack& stack, int nArgs);
Object* isStringFunction(ObjectStack& stack, int nArgs);
Object* isNumberFunction(ObjectStack& stack, int nArgs);
Object* isConsFunction(ObjectStack& stack, int nArgs);
Object* isFunctionFunction(ObjectStack& stack, int nArgs);
Object* isUserFunctionFunction(ObjectStack& stack, int nArgs);
Object* isBoolFunction(ObjectStack& stack, int nArgs);

// USER DEFINED FUNCTIONS

}  // namespace scm
