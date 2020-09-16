#include "scheme.hpp"
#include <iostream>

namespace scm {

// getter functions
ObjectTypeTag getTag(Object* obj)
{
  return obj->tag;
}

std::string getStringValue(Object* obj)
{
  if (!hasTag(obj, TAG_STRING) && !hasTag(obj, TAG_SYMBOL)) {
    schemeThrow("object has no string value that could be gotten");
  }
  return std::get<std::string>(obj->value);
}

int getIntValue(Object* obj)
{
  if (!hasTag(obj, TAG_INT)) {
    schemeThrow("object has no integer value that could be gotten");
  }
  return std::get<int>(obj->value);
}

double getFloatValue(Object* obj)
{
  if (!hasTag(obj, TAG_FLOAT)) {
    schemeThrow("object has no float value that could be gotten");
  }
  return std::get<double>(obj->value);
}

ConsValue getCons(Object* obj)
{
  if (!hasTag(obj, TAG_CONS)) {
    schemeThrow("tried to get consvalue from non-cons object: " + toString(obj));
  }
  return std::get<ConsValue>(obj->value);
}

Object* getCar(Object* obj)
{
  auto cons{getCons(obj)};
  return cons.car;
}

Object* getCdr(Object* obj)
{
  auto cons{getCons(obj)};
  return cons.cdr;
}

FunctionTag getBuiltinFuncTag(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).funcTag;
}

std::string getBuiltinFuncName(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).name;
}

int getBuiltinFuncNArgs(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).nArgs;
}

std::string getBuiltinFuncHelpText(Object* obj)
{
  if (!isOneOf(obj, {TAG_FUNC_BUILTIN, TAG_SYNTAX})) {
    schemeThrow("not a builtin function!");
  }
  return std::get<FuncValue>(obj->value).helpText;
}

Object* getUserFunctionArgList(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).argList;
}

Object* getUserFunctionBodyList(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).bodyList;
}

Environment* getUserFunctionParentEnv(Object* obj)
{
  if (!hasTag(obj, TAG_FUNC_USER)) {
    schemeThrow("not a user function!");
  }
  return std::get<UserFuncValue>(obj->value).env;
}

// Bool operations
bool isString(Object* obj)
{
  return hasTag(obj, TAG_STRING);
}

bool isNumeric(Object* obj)
{
  return (hasTag(obj, TAG_INT) || hasTag(obj, TAG_FLOAT));
}

bool isFloatingPoint(Object* obj)
{
  return hasTag(obj, TAG_FLOAT);
}

bool hasTag(Object* obj, ObjectTypeTag tag)
{
  return (getTag(obj) == tag);
}

bool isOneOf(Object* obj, std::vector<ObjectTypeTag> validTypes)
{
  auto lambda = [obj](ObjectTypeTag tag) { return hasTag(obj, tag); };
  return std::any_of(validTypes.begin(), validTypes.end(), lambda);
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

// other helper functions

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

}  // namespace scm