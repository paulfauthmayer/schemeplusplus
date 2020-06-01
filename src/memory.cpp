#include "scheme.hpp"

scmObject SCM_NIL;
scmObject SCM_TRUE;
scmObject SCM_FALSE;

scmObject scm_newSingleton(scmTag tag)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));
  p->tag = tag;

  return p;
}

void initializeSingletons()
{
  SCM_NIL = scm_newSingleton(TAG_NIL);
  SCM_TRUE = scm_newSingleton(TAG_TRUE);
  SCM_FALSE = scm_newSingleton(TAG_FALSE);
}

scmObject scm_newInteger(int iVal)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));
  p->tag = TAG_INTEGER;
  p->u.intValue = iVal;

  return p;
}

scmObject scm_newString(char* characters)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));
  p->tag = TAG_STRING;
  p->u.stringValue = characters;

  return p;
}

scmObject scm_newSymbol(char* characters)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));
  p->tag = TAG_SYMBOL;
  p->u.symbolValue = characters;

  return p;
}

scmObject scm_newCons(scmObject car, scmObject cdr)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));

  p->tag = TAG_CONS;
  p->u.consValue.car = car;
  p->u.consValue.cdr = cdr;

  return p;
}