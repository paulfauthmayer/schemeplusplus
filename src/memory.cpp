#include "memory.hpp"
#include "scheme.hpp"

scm::Object* SCM_NIL;
scm::Object* SCM_TRUE;
scm::Object* SCM_FALSE;

void initializeSingletons()
{
  SCM_NIL = newSingleton(scm::TAG_NIL);
  SCM_TRUE = newSingleton(scm::TAG_TRUE);
  SCM_FALSE = newSingleton(scm::TAG_FALSE);
}

scm::Object* newSingleton(scm::ObjectTypeTag type)
{
  scm::Object* obj{new scm::Object(type)};
  return obj;
}

scm::Object* newInteger(int value)
{
  scm::Object* obj{new scm::Object(scm::TAG_INT)};
  obj->value = value;
  return obj;
}

scm::Object* newFloat(double value)
{
  scm::Object* obj{new scm::Object(scm::TAG_FLOAT)};
  obj->value = value;
  return obj;
}

scm::Object* newString(std::string value)
{
  scm::Object* obj{new scm::Object(scm::TAG_STRING)};
  obj->value = value;
  return obj;
}

scm::Object* newSybmol(std::string value)
{
  scm::Object* obj{new scm::Object(scm::TAG_SYMBOL)};
  obj->value = value;
  return obj;
}
