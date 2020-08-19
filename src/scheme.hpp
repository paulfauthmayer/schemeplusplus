#pragma once
#include <iostream>
#include <string>
#include <variant>

namespace scm {

enum ObjectTypeTag {
  TAG_INT = 1,
  TAG_FLOAT,
  TAG_STRING,
  TAG_SYMBOL,
  TAG_CONS,
  TAG_NIL,
  TAG_TRUE,
  TAG_FALSE,
  TAG_ENV,
  TAG_FUNC_BUILTIN,
  TAG_FUNC_USER,
  TAG_SYNTAX,
};

struct Object;
struct ConsValue {
  Object* car;
  Object* cdr;
};

struct Object {
  ObjectTypeTag tag;
  std::variant<int, double, std::string, ConsValue> value;
  Object(ObjectTypeTag tag) : tag(tag){};
};

inline std::string toString(scm::Object* obj);
static std::string consToString(scm::Object* cons, std::string& str)
{
  scm::Object *car, *cdr;
  car = std::get<ConsValue>(cons->value).car;
  cdr = std::get<ConsValue>(cons->value).cdr;
  str += toString(car) + " ";
  if (cdr->tag == TAG_NIL)
    return str + ")";
  return consToString(cdr, str);
}

inline std::string toString(scm::Object* obj)
{
  std::string consStart;
  switch (obj->tag) {
    case TAG_INT:
      return std::to_string(std::get<int>(obj->value));
      break;
    case TAG_FLOAT:
      return std::to_string(std::get<double>(obj->value));
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
    default:
      return "{{NOT YET IMPLEMENTED}}";
      break;
  }
}

}  // namespace scm
