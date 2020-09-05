#include "memory.hpp"
#include <loguru.hpp>
#include "scheme.hpp"

namespace scm {

// these are the language specific singletons
// we don't want more than one of each of these
Object* SCM_NIL;
Object* SCM_VOID;
Object* SCM_EOF;
Object* SCM_TRUE;
Object* SCM_FALSE;

/**
 * Sets up all singleton objects
 */
void initializeSingletons()
{
  SCM_NIL = newSingleton(TAG_NIL);
  SCM_VOID = newSingleton(TAG_VOID);
  SCM_EOF = newSingleton(TAG_EOF);
  SCM_TRUE = newSingleton(TAG_TRUE);
  SCM_FALSE = newSingleton(TAG_FALSE);
}

/**
 * Create a new Singleton of the specified type
 * @param type the type of the singleton
 * @returns a pointer to the allocated object
 */
Object* newSingleton(ObjectTypeTag type)
{
  Object* obj{new Object(type)};
  return obj;
}

/**
 * Create a new scheme integer
 * @param value the value of the integer
 * @returns a pointer to the allocated object
 */
Object* newInteger(int value)
{
  Object* obj{new Object(TAG_INT)};
  obj->value = value;
  return obj;
}

/**
 * Create a new scheme float
 * @param value the value of the float
 * @returns a pointer to the allocated object
 */
Object* newFloat(double value)
{
  Object* obj{new Object(TAG_FLOAT)};
  obj->value = value;
  return obj;
}

/**
 * Create a new scheme string
 * @param value the value of the string
 * @returns a pointer to the allocated object
 */
Object* newString(std::string value)
{
  Object* obj{new Object(TAG_STRING)};
  obj->value = value;
  return obj;
}

/**
 * Create a new scheme symbol
 * @param value the name of the symbol
 * @returns a pointer to the allocated object
 */
Object* newSymbol(std::string value)
{
  Object* obj{new Object(TAG_SYMBOL)};
  obj->value = value;
  return obj;
}

/**
 * Create a new scheme cons object
 * @param car the first element of the cons
 * @param cdr the rest of the cons
 * @returns a pointer to the allocated object
 */
Object* newCons(Object* car, Object* cdr)
{
  Object* obj{new Object(TAG_CONS)};
  obj->value = ConsValue{car, cdr};
  return obj;
}

/**
 * Create a new scheme builtin function object
 * @param name the name of the function
 * @param numArgs the amount of arguments required by the function
 * @param funcTag the tag of the function for identification purposes
 * @param helpText the text diplayed when requested help for
 * @see defineNewBuiltinFunction
 * @see helpSyntax
 * @returns a pointer to the allocated object
 */
Object* newBuiltinFunction(std::string name, int numArgs, FunctionTag funcTag, std::string helpText)
{
  Object* obj{new Object(TAG_FUNC_BUILTIN)};
  obj->value = FuncValue{"primitive:" + name, numArgs, funcTag, helpText};
  return obj;
};

/**
 * Create a new scheme builtin syntax object
 * @param name the name of the syntax
 * @param numArgs the amount of arguments required by the syntax
 * @param funcTag the tag of the syntax for identification purposes
 * @param helpText the text diplayed when requested help for
 * @see defineNewSyntax
 * @see helpSyntax
 * @returns a pointer to the allocated object
 */
Object* newSyntax(std::string name, int numArgs, FunctionTag funcTag, std::string helpText)
{
  Object* obj{new Object(TAG_SYNTAX)};
  obj->value = FuncValue{"syntax:" + name, numArgs, funcTag, helpText};
  return obj;
}

/**
 * Create a new user defined function object
 * @param argList a cons with all arguments required by the function
 * @param bodyList a cons of one or more expressions to be evaluated
 * @param homeEnv the home environment of the function, arguments and variables are defined in here
 * @returns a pointer to the allocated object
 */
Object* newUserFunction(Object* argList, Object* bodyList, Environment& homeEnv)
{
  Object* obj{new Object(TAG_FUNC_USER)};
  obj->value = UserFuncValue{argList, bodyList, &homeEnv};
  return obj;
}
}  // namespace scm
