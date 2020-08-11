#include "memory.hpp"
#include "scheme.hpp"

scm::Object SCM_NIL;
scm::Object SCM_TRUE;
scm::Object SCM_FALSE;

void initializeSingletons()
{
  SCM_NIL = newSingleton(scm::TAG_NIL);
  SCM_TRUE = newSingleton(scm::TAG_TRUE);
  SCM_FALSE = newSingleton(scm::TAG_FALSE);
}

scm::Object allocateScmObject()
{
  scm::Object obj = static_cast<scm::Object>(malloc(sizeof(struct scm::ObjectStruct)));
  return obj;
}

scm::Object newSingleton(scm::Tag tag)
{
  scm::Object obj = allocateScmObject();
  obj->tag = tag;
  return obj;
}

scm::Object newInteger(int intValue)
{
  scm::Object obj = allocateScmObject();
  obj->tag = scm::TAG_INT;
  obj->u.intValue = intValue;
  return obj;
}

scm::Object newFloat(double floatValue)
{
  scm::Object obj = allocateScmObject();
  obj->tag = scm::TAG_FLOAT;
  obj->u.floatValue = floatValue;
  return obj;
}

scm::Object newString(std::string stringValue)
{
  scm::Object obj = allocateScmObject();
  obj->tag = scm::TAG_STRING;
  obj->u.stringValue = stringValue;
  return obj;
}

scm::Object newSybmol(std::string name)
{
  scm::Object obj{allocateScmObject()};
  obj->tag = scm::TAG_SYMBOL;
  obj->u.symbolValue = name;
  return obj;
}
