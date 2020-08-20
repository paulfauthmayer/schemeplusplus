#pragma once
#include <exception>
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

enum FunctionTag {
  SYNTAX_QUOTE,
  SYNTAX_LAMBDA,
  SYNTAX_DEFINE,
  SYNTAX_IF,
  SYNTAX_SET,
  SYNTAX_BEGIN,

  FUNC_ADD,
  FUNC_SUB,
  FUNC_MULT,
  FUNC_CONS,
  FUNC_CAR,
  FUNC_CDR,
  FUNC_EQUAL,
  FUNC_GT,
  FUNC_LT,
  FUNC_LIST,
  FUNC_DISPLAY,
  FUNC_IS_STRING,
  FUNC_IS_NUMBER,
  FUNC_IS_CONS,
  FUNC_IS_FUNC,
  FUNC_IS_USERFUNC,
  FUNC_IS_BOOL,
};

struct Object;
struct ConsValue {
  Object* car;
  Object* cdr;
};
struct FuncValue {
  std::string name;
  int nArgs;
  FunctionTag funcTag;
};

struct Object {
  ObjectTypeTag tag;
  std::variant<int, double, std::string, ConsValue, FuncValue> value;
  Object(ObjectTypeTag tag) : tag(tag){};
};

struct schemeException : public std::exception {
  std::string m_error;
  schemeException(std::string error) : m_error{error} {}
  const char* what() const noexcept { return m_error.c_str(); }
};

// Forward Declarations
std::string getStringValue(Object* obj);
int getIntValue(Object* obj);
double getFloatValue(Object* obj);
ConsValue getCons(Object* obj);
Object* getCar(Object* obj);
Object* getCdr(Object* obj);
FunctionTag getBuiltinFuncTag(Object* obj);
std::string getBuiltinFuncName(Object* obj);
int getBuiltinFuncNArgs(Object* obj);
bool hasTag(Object* obj, ObjectTypeTag tag);
bool isString(Object* obj);
bool isNumeric(Object* obj);
bool isFloatingPoint(Object* obj);
std::string toString(scm::Object* obj);
static std::string consToString(scm::Object* cons, std::string& str);

}  // namespace scm
