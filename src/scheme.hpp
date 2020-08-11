#pragma once
#include <iostream>

namespace scm {

enum ObjectType {
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

using Tag = enum ObjectType;
using Object = struct ObjectStruct*;

struct ObjectStruct {
  Tag tag;
  union {
    int intValue;
    double floatValue;
    std::string stringValue;
    std::string symbolValue;
    struct {
      Object car;
      Object cdr;
    } consValue;
  } u;
};

}  // namespace scm
