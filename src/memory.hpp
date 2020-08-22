#pragma once
#include "scheme.hpp"

namespace scm {

void initializeSingletons();
Object* newSingleton(ObjectTypeTag tag);
Object* newInteger(int value);
Object* newFloat(double value);
Object* newString(std::string value);
Object* newEnv();
Object* newSymbol(std::string value);
Object* newCons(Object* car, Object* cdr);
Object* newSyntax(std::string name, int numArgs, FunctionTag funcTag);
Object* newBuiltinFunction(std::string name, int numArgs, FunctionTag funcTag);

extern Object* SCM_NIL;
extern Object* SCM_TRUE;
extern Object* SCM_FALSE;

}  // namespace scm