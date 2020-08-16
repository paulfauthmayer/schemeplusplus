#pragma once
#include <iostream>
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
struct consValue {
  Object* car;
  Object* cdr;
};

struct Object {
  ObjectTypeTag tag;
  std::variant<int, double, std::string, consValue> value;

  Object(ObjectTypeTag tag) : tag(tag){};
};

}  // namespace scm
