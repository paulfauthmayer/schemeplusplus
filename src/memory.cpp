#include "memory.hpp"
#include <loguru.hpp>
#include "scheme.hpp"

namespace scm {

Object* SCM_NIL;
Object* SCM_VOID;
Object* SCM_EOF;
Object* SCM_TRUE;
Object* SCM_FALSE;

void initializeSingletons()
{
  SCM_NIL = newSingleton(TAG_NIL);
  SCM_VOID = newSingleton(TAG_VOID);
  SCM_EOF = newSingleton(TAG_EOF);
  SCM_TRUE = newSingleton(TAG_TRUE);
  SCM_FALSE = newSingleton(TAG_FALSE);
}

Object* newSingleton(ObjectTypeTag type)
{
  Object* obj{new Object(type)};
  return obj;
}

Object* newInteger(int value)
{
  Object* obj{new Object(TAG_INT)};
  obj->value = value;
  return obj;
}

Object* newFloat(double value)
{
  Object* obj{new Object(TAG_FLOAT)};
  obj->value = value;
  return obj;
}

Object* newString(std::string value)
{
  Object* obj{new Object(TAG_STRING)};
  obj->value = value;
  return obj;
}

Object* newSymbol(std::string value)
{
  Object* obj{new Object(TAG_SYMBOL)};
  obj->value = value;
  return obj;
}

Object* newCons(Object* car, Object* cdr)
{
  Object* obj{new Object(TAG_CONS)};
  obj->value = ConsValue{car, cdr};
  return obj;
}

Object* newBuiltinFunction(std::string name, int numArgs, FunctionTag funcTag)
{
  Object* obj{new Object(TAG_FUNC_BUILTIN)};
  obj->value = FuncValue{"primitive:" + name, numArgs, funcTag};
  return obj;
};

Object* newSyntax(std::string name, int numArgs, FunctionTag funcTag)
{
  Object* obj{new Object(TAG_SYNTAX)};
  obj->value = FuncValue{"syntax:" + name, numArgs, funcTag};
  return obj;
}

Object* newUserFunction(Object* argList, Object* bodyList, Environment& homeEnv)
{
  Object* obj{new Object(TAG_FUNC_USER)};
  obj->value = UserFuncValue{argList, bodyList, &homeEnv};
  return obj;
}
}  // namespace scm
