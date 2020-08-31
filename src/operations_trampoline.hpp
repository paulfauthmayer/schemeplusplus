#pragma once
#include <iostream>
#include <stack>
#include "scheme.hpp"
namespace scm {
namespace trampoline {

// SYNTAX
Continuation* defineSyntax();
Continuation* setSyntax();
Continuation* quoteSyntax();
Continuation* ifSyntax();
Continuation* beginSyntax();
Continuation* lambdaSyntax();
Continuation* helpSyntax();

// BUILTIN FUNCTIONS
Continuation* addFunction();
Continuation* subFunction();
Continuation* multFunction();
Continuation* divFunction();
Continuation* eqFunction();
// Continuation* equalFunction();
Continuation* equalStringFunction();
Continuation* equalNumberFunction();
Continuation* greaterThanFunction();
Continuation* lesserThanFunction();
Continuation* consFunction();
Continuation* carFunction();
Continuation* cdrFunction();
Continuation* listFunction();
Continuation* displayFunction();
Continuation* functionBodyFunction();
Continuation* functionArglistFunction();
Continuation* isStringFunction();
Continuation* isNumberFunction();
Continuation* isConsFunction();
Continuation* isBuiltinFunctionFunction();
Continuation* isUserFunctionFunction();
Continuation* isBoolFunction();

// USER DEFINED FUNCTIONS

}  // namespace trampoline
}  // namespace scm
