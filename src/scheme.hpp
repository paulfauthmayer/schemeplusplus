#pragma once
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include <stack>
#include <string>
#include <variant>
#include "garbage_collection.hpp"

namespace scm {

/**
 * The types our objects can be
 */
enum ObjectTypeTag {
  TAG_INT = 1,
  TAG_FLOAT,
  TAG_STRING,
  TAG_SYMBOL,
  TAG_CONS,
  TAG_NIL,
  TAG_TRUE,
  TAG_FALSE,
  TAG_FUNC_BUILTIN,
  TAG_FUNC_USER,
  TAG_SYNTAX,
  TAG_VOID,
  TAG_EOF,
};

/**
 * Identifies the individual builtin functions and syntax
 */
enum FunctionTag {
  SYNTAX_QUOTE,
  SYNTAX_LAMBDA,
  SYNTAX_DEFINE,
  SYNTAX_IF,
  SYNTAX_SET,
  SYNTAX_BEGIN,
  SYNTAX_HELP,

  FUNC_ADD,
  FUNC_SUB,
  FUNC_MULT,
  FUNC_DIV,
  FUNC_CONS,
  FUNC_CAR,
  FUNC_CDR,
  FUNC_EQ,
  FUNC_EQUAL_NUMBER,
  FUNC_EQUAL_STRING,
  FUNC_GT,
  FUNC_LT,
  FUNC_LIST,
  FUNC_DISPLAY,
  FUNC_FUNCTION_BODY,
  FUNC_FUNCTION_ARGLIST,
  FUNC_IS_STRING,
  FUNC_IS_NUMBER,
  FUNC_IS_CONS,
  FUNC_IS_FUNC,
  FUNC_IS_USERFUNC,
  FUNC_IS_BOOL,
};

// forward declarations required for Object Class
struct Object;
class Environment;

// a Cons consists of at least one element and element after that
struct ConsValue {
  Object* car;
  Object* cdr;
};
// the value of a builtin function or syntax
struct FuncValue {
  std::string name;
  int nArgs;
  FunctionTag funcTag;
  std::string helpText;
};
// the value of a user defined function
struct UserFuncValue {
  Object* argList;
  Object* bodyList;
  Environment* env;
};

/**
 * The base class of every scheme object we use.
 * Central point of this interpreter
 */
struct Object : public Collectable {
  ObjectTypeTag tag;
  // can hold multiple different values
  std::variant<int, double, std::string, ConsValue, FuncValue, UserFuncValue> value;
  Object(ObjectTypeTag tag) : tag(tag){};
  ~Object(){};
};

/**
 * A custom exception thrown when something goes wrong with our interpreter.
 */
class schemeException : public std::runtime_error {
 private:
  std::string m_error;

 public:
  schemeException(const std::string& arg, const char* file, int line) : std::runtime_error(arg)
  {
    std::ostringstream o;
    // TODO: make this work for windows as well
    std::string filePath{file};
    std::vector<std::string> v;
    std::regex re(R"([^\\/]+)");
    for (std::sregex_iterator i = std::sregex_iterator(filePath.begin(), filePath.end(), re);
         i != std::sregex_iterator();
         i++) {
      v.push_back(std::smatch(*i).str());
    }

    o << "[ERROR:" << v.back() << ":" << line << "] " << arg;
    m_error = o.str();
  }
  ~schemeException() throw() {}
  const char* what() const throw() { return m_error.c_str(); }
};

// a macro to also print the line and file of where the exception was thrown
#define schemeThrow(arg) throw schemeException(arg, __FILE__, __LINE__);

// Forward Declarations
ObjectTypeTag getTag(Object* obj);
std::string getStringValue(Object* obj);
int getIntValue(Object* obj);
double getFloatValue(Object* obj);
ConsValue getCons(Object* obj);
Object* getCar(Object* obj);
Object* getCdr(Object* obj);
FunctionTag getBuiltinFuncTag(Object* obj);
std::string getBuiltinFuncName(Object* obj);
int getBuiltinFuncNArgs(Object* obj);
std::string getBuiltinFuncHelpText(Object* obj);
Object* getUserFunctionBodyList(Object* obj);
Object* getUserFunctionArgList(Object* obj);
std::string getBuiltinFuncHelpText(Object* obj);
Environment* getUserFunctionParentEnv(Object* obj);
bool hasTag(Object* obj, ObjectTypeTag tag);
bool isString(Object* obj);
bool isNumeric(Object* obj);
bool isFloatingPoint(Object* obj);
bool isOneOf(Object* obj, std::vector<ObjectTypeTag> validTypes);
std::string tagToString(ObjectTypeTag tag);
std::string toString(scm::Object* obj);
static std::string consToString(scm::Object* cons, std::string& str);
std::string prettifyUserFunction(Object* func);

// Macros and Typedefs

// what this is supposed to do: *function -> *function
// this is used in our trampoline (see evaluation.cpp)
// VoidoPtrFunc is a function returning a void pointer
using VoidPtrFunc = void*();
// Continuation is a function returning a pointer to a function
using Continuation = VoidPtrFunc*();

// in theory, this should do the same thing as above
// but it doesn't. why? If you know why please let me know :)
// using VoidPtrFunc = std::function<void*()>;
// using Continuation = std::function<VoidPtrFunc*()>;

using ObjectVec = std::vector<Object*>;
using ObjectStack = std::stack<Object*>;
using FunctionStack = std::stack<Continuation*>;

// logging activation
extern bool LOG_GARBAGE_COLLECTION;
extern bool LOG_TRAMPOLINE_TRACE;
extern bool LOG_STACK_TRACE;
extern bool LOG_PARSER;
extern bool LOG_MEMORY;
extern bool LOG_TESTS;
extern bool LOG_ENVIRONMENT;
extern bool LOG_EVALUATION;
}  // namespace scm
