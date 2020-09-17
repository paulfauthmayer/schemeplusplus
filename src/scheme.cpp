#include "scheme.hpp"
#include <iostream>

namespace scm {

// getter functions
/**
 * Returns the ObjectTypeTag of an object.
 * @param obj the object from which to read the tag
 * @returns the tag of the object
 */
ObjectTypeTag getTag(Object* obj)
{
  return obj->tag;
}

/**
 * Returns the string value of an object, if applicable.
 * @param obj the object from which to read the string value
 * @throw schemeException on invalid object
 * @returns the string value of the object
 */
std::string getStringValue(Object* obj)
{
  if (!hasTag(obj, TAG_STRING) && !hasTag(obj, TAG_SYMBOL)) {
    schemeThrow("object has no string value that could be gotten");
  }
  return std::get<std::string>(obj->value);
}

/**
 * Returns the integer value of an object, if applicable.
 * @param obj the object from which to read the integer value
 * @throw schemeException on invalid object
 * @returns the integer value of the object
 */
int getIntValue(Object* obj)
{
  if (!hasTag(obj, TAG_INT)) {
    schemeThrow("object has no integer value that could be gotten");
  }
  return std::get<int>(obj->value);
}

/**
 * Returns the float value of an object, if applicable.
 * @param obj the object from which to read the float value
 * @throw schemeException on invalid object
 * @returns the float value of the object
 */
double getFloatValue(Object* obj)
{
  if (!hasTag(obj, TAG_FLOAT)) {
    schemeThrow("object has no float value that could be gotten");
  }
  return std::get<double>(obj->value);
}

/**
 * Returns the cons value of an object, if applicable.
 * @param obj the object from which to read the cons value
 * @throw schemeException on invalid object
 * @returns the cons value of the object
 */
ConsValue getCons(Object* obj)
{
  if (!hasTag(obj, TAG_CONS)) {
    schemeThrow("tried to get consvalue from non-cons object: " + toString(obj));
  }
  return std::get<ConsValue>(obj->value);
}

/**
 * Returns the car of a cons object
 * @param obj the cons object from which to read the car
 * @throw schemeException on invalid object
 * @returns the car of the cons object
 */
Object* getCar(Object* obj)
{
  auto cons{getCons(obj)};
  return cons.car;
}

/**
 * Returns the cdr of a cons object
 * @param obj the cons object from which to read the cdr
 * @throw schemeException on invalid object
 * @returns the cdr of the cons object
 */
Object* getCdr(Object* obj)
{
  auto cons{getCons(obj)};
  return cons.cdr;
}

/**
 * Returns the function type tag of a scheme function or syntax object
 * @param obj the object from which to read the tag
 * @throw schemeException on non-syntax ort non-builtin-function
 * @returns the function tag of the object
 */
FunctionTag getBuiltinFuncTag(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).funcTag;
}

/**
 * Returns the name of a scheme function or syntax object
 * @param obj the object from which to read the name
 * @throw schemeException on non-syntax ort non-builtin-function
 * @returns the function name of the object
 */
std::string getBuiltinFuncName(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).name;
}

/**
 * Returns the number of arguments of a scheme function or syntax object
 * @param obj the object from which to read the number of arguments
 * @throw schemeException on non-syntax ort non-builtin-function
 * @returns the number of arguments of the object
 */
int getBuiltinFuncNArgs(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).nArgs;
}

/**
 * Returns the help text of a scheme function or syntax object
 * @param obj the object from which to read the help text
 * @throw schemeException on non-syntax ort non-builtin-function
 * @returns the help text of the function
 */
std::string getBuiltinFuncHelpText(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).helpText;
}

/**
 * Returns the argument list of a user defined function.
 * @param obj the user defined function object from which to get the argument list
 * @throw schemeException if obj isn't a user defined function
 * @returns the argument list of the object
 */
Object* getUserFunctionArgList(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).argList;
}

/**
 * Returns the body list of a user defined function.
 * @param obj the user defined function object from which to get the body list
 * @throw schemeException if obj isn't a user defined function
 * @returns the body list of the object
 */
Object* getUserFunctionBodyList(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).bodyList;
}

/**
 * Returns the parent environment of a user defined function.
 * @param obj the user defined function object from which to get the parent environment
 * @throw schemeException if obj isn't a user defined function
 * @returns the parent environment of the object
 */
Environment* getUserFunctionParentEnv(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).env;
}

// Bool operations
/**
 * Is the passed object a string?
 * @param obj the object to be checked
 * @returns true if the object is a string, false otherwise
 */
bool isString(Object* obj)
{
  return hasTag(obj, TAG_STRING);
}

/**
 * Is the passed object a number?
 * @param obj the object to be checked
 * @returns true if the object is a number, false otherwise
 */
bool isNumeric(Object* obj)
{
  return (hasTag(obj, TAG_INT) || hasTag(obj, TAG_FLOAT));
}

/**
 * Is the passed object a float?
 * @param obj the object to be checked
 * @returns true if the object is a float, false otherwise
 */
bool isFloatingPoint(Object* obj)
{
  return hasTag(obj, TAG_FLOAT);
}

/**
 * Does the object has a specific tag?
 * @param obj the object to be checked
 * @returns true if the has the same tag, false otherwise
 */
bool hasTag(Object* obj, ObjectTypeTag tag)
{
  return (getTag(obj) == tag);
}

/**
 * Does the object is one of a list of types
 * @param obj the object to be checked
 * @returns true if the has the same tag, false otherwise
 */
bool isOneOf(Object* obj, std::vector<ObjectTypeTag> validTypes)
{
  auto lambda = [obj](ObjectTypeTag tag) { return hasTag(obj, tag); };
  return std::any_of(validTypes.begin(), validTypes.end(), lambda);
}

// other helper functions

/**
 * Returns a readable representation of a Object Tag
 * @param tag the tag to represent
 * @returns the string representation of the tag
 */
std::string tagToString(ObjectTypeTag tag)
{
  switch (tag) {
    case TAG_INT:
      return "integer";
      break;
    case TAG_FLOAT:
      return "float";
      break;
    case TAG_STRING:
      return "string";
      break;
    case TAG_SYMBOL:
      return "symbol";
      break;
    case TAG_CONS:
      return "cons";
      break;
    case TAG_NIL:
      return "cons";
      break;
    case TAG_TRUE:
      return "true";
      break;
    case TAG_FALSE:
      return "false";
      break;
    case TAG_FUNC_BUILTIN:
      return "builtin function";
      break;
    case TAG_FUNC_USER:
      return "user function";
      break;
    case TAG_SYNTAX:
      return "syntax";
      break;
    case TAG_VOID:
      return "void";
      break;
    default:
      return "unrecognized name";
  };
}

/**
 * Returns a readable representation of a cons object, only to be used by toString.
 * @param tag the cons object to represent
 * @returns the string representation of the object
 */
static std::string consToString(scm::Object* cons, std::string& str)
{
  scm::Object *car, *cdr;
  car = getCar(cons);
  cdr = getCdr(cons);
  str += toString(car) + " ";
  if (hasTag(cdr, TAG_CONS)) {
    return consToString(cdr, str);
  }
  else if (cdr->tag == TAG_NIL) {
    return str + ")";
  }
  else {
    return str + ". " + toString(cdr) + ')';
  }
}

/**
 * Returns a readable representation of an object.
 * @param tag the object to represent
 * @returns the string representation of the object
 */
std::string toString(Object* obj)
{
  std::string consStart;
  switch (obj->tag) {
    case TAG_INT:
      return std::to_string(std::get<int>(obj->value));
      break;
    case TAG_FLOAT:
      return std::to_string(std::get<double>(obj->value));
      break;
    case TAG_STRING:
      return '"' + std::get<std::string>(obj->value) + '"';
      break;
    case TAG_SYMBOL:
      return std::get<std::string>(obj->value);
      break;
    case TAG_NIL:
      return "()";
      break;
    case TAG_FALSE:
      return "#f";
      break;
    case TAG_TRUE:
      return "#t";
      break;
    case TAG_CONS:
      consStart = "( ";
      return consToString(obj, consStart);
      break;
    case TAG_FUNC_BUILTIN:
      return "#<" + getBuiltinFuncName(obj) + '>';
      break;
    case TAG_FUNC_USER:
      return "(lambda " + toString(getUserFunctionArgList(obj)) + " " +
             toString(getUserFunctionBodyList(obj)) + ")";
    case TAG_SYNTAX:
      return "#<" + getBuiltinFuncName(obj) + '>';
    case TAG_EOF:
      return "finished";
    case TAG_VOID:
      return "V̱̲̠̹̪́ͨ̇̄̏ͤ́̊͌Ơ̶̸̖̮̙̘̻̘͇̘ͭ͋͛̾̇Į̶̯ͦ̃́̅͗D̵͔̯̰̞͔̖̞̣͌ͪ̓ͨ͋";
    default:
      return "{{TO STRING NOT YET IMPLEMENTED FOR TAG " + std::to_string(obj->tag) + "}}";
      break;
  }
}

/**
 * Return a readable representation of a user defined function, mainly used in
 * help text.
 * @param func the function to be made readable
 * @returns a string with the readable representation
 */
std::string prettifyUserFunction(Object* func)
{
  if (!hasTag(func, TAG_FUNC_USER))
    schemeThrow("not a user defined function!");

  // define container for result and tokens
  std::string pretty;
  std::string token;

  // create streamstring to read from
  std::stringstream ss{toString(func)};

  // keep track of number of open parantheses for indentation purposes
  int indentCount{};
  int closeCount;
  bool discardFirst{true};

  while (std::getline(ss, token, '(')) {
    // first split is always empty and can be discarded
    if (discardFirst) {
      discardFirst = false;
      continue;
    }
    // indent by proper amount of spaces
    for (int i{}; i < indentCount; i++) {
      std::cout << "  ";
    }
    // print line
    std::cout << '(' << token << '\n';

    // calculate indentation for next line
    indentCount++;
    closeCount = std::count(token.begin(), token.end(), ')');
    indentCount -= closeCount;
  }
  return pretty;
}

// enable logging for certain parts of the interpreter
bool LOG_ENVIRONMENT = 0;
bool LOG_EVALUATION = 0;
bool LOG_MEMORY = 0;
bool LOG_PARSER = 0;
bool LOG_STACK_TRACE = 0;
bool LOG_TESTS = 1;
bool LOG_TRAMPOLINE_TRACE = 0;
bool LOG_GARBAGE_COLLECTION = 0;

}  // namespace scm