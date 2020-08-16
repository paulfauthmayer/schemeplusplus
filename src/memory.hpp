#pragma once
#include "scheme.hpp"

void initializeSingletons();
scm::Object* newSingleton(scm::ObjectTypeTag tag);
scm::Object* newInteger(int value);
scm::Object* newFloat(double value);
scm::Object* newString(std::string value);
scm::Object* newEnv();
scm::Object* newSybmol(std::string value);

extern scm::Object* SCM_NIL;
extern scm::Object* SCM_TRUE;
extern scm::Object* SCM_FALSE;