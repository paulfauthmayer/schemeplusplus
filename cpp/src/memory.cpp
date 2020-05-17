#include "scheme.hpp"

scmObject scm_newInteger(int iVal)
{
  scmObject p = (scmObject)malloc(sizeof(struct scmObjectStruct));
  p->tag = TAG_INTEGER;
  p->u.intValue = iVal;

  return p;
}