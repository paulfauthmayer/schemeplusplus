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
  return std::get<std::string>(obj->value);
}

int getIntValue(Object* obj)
{
  return std::get<int>(obj->value);
}

double getFloatValue(Object* obj)
{
  return std::get<double>(obj->value);
}

ConsValue getCons(Object* obj)
{
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
  if (obj->tag != TAG_FUNC_BUILTIN)
    throw(schemeException("not a builtin function!"));
  return std::get<FuncValue>(obj->value).funcTag;
}

std::string getBuiltinFuncName(Object* obj)
{
  if (obj->tag != TAG_FUNC_BUILTIN)
    throw(schemeException("not a builtin function!"));
  return std::get<FuncValue>(obj->value).name;
}

int getBuiltinFuncNArgs(Object* obj)
{
  if (obj->tag != TAG_FUNC_BUILTIN)
    throw(schemeException("not a builtin function!"));
  return std::get<FuncValue>(obj->value).nArgs;
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

// other helper functions

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
      return "()'";
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
    case TAG_SYNTAX:
      return "#<" + getBuiltinFuncName(obj) + '>';
    default:
      return "{{TO STRING NOT YET IMPLEMENTED FOR TAG " + std::to_string(obj->tag) + "}}";
      break;
  }
}
}  // namespace scm