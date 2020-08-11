#pragma once
#include "scheme.hpp"

void initializeSingletons();
scm::Object newSingleton(scm::Tag);
scm::Object newInteger(int intValue);
scm::Object newFloat(double floatValue);
scm::Object newString(std::string stringValue);
scm::Object newEnv();
scm::Object newSybmol(std::string name);

extern scm::Object SCM_NIL;
extern scm::Object SCM_TRUE;
extern scm::Object SCM_FALSE;