#ifndef MEMORY_HPP
#define MEMORY_HPP
#include "scheme.hpp"

scmObject scm_newSingleton(scmTag);
scmObject scm_newInteger(int);
scmObject scm_newString(char*);
scmObject scm_newSymbol(char*);
scmObject scm_newCons(scmObject car, scmObject cdr);

void initializeSingletons();

extern scmObject SCM_NIL;
extern scmObject SCM_TRUE;
extern scmObject SCM_FALSE;

#endif