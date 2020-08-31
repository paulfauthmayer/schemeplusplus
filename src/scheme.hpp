#pragma once
#include <exception>
#include <iostream>
#include <regex>
#include <sstream>
#include <stack>
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
  TAG_VOID,
  TAG_EOF,
};

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
  FUNC_MOD,
  FUNC_CONS,
  FUNC_CAR,
  FUNC_CDR,
  FUNC_EQ,
  FUNC_EQUAL,
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

struct Object;
struct ConsValue {
  Object* car;
  Object* cdr;
};
struct FuncValue {
  std::string name;
  int nArgs;
  FunctionTag funcTag;
  std::string helpText;
};

class Environment;  // forward definition for environment.hpp
struct UserFuncValue {
  Object* argList;
  Object* bodyList;
  Environment* env;
};

struct Object {
  ObjectTypeTag tag;
  std::variant<int, double, std::string, ConsValue, FuncValue, UserFuncValue> value;
  Object(ObjectTypeTag tag) : tag(tag){};
};

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
Environment* getUserFunctionParentEnv(Object* obj);
bool hasTag(Object* obj, ObjectTypeTag tag);
bool isString(Object* obj);
bool isNumeric(Object* obj);
bool isFloatingPoint(Object* obj);
bool isOneOf(Object* obj, std::vector<ObjectTypeTag> validTypes);
std::string toString(scm::Object* obj);
static std::string consToString(scm::Object* cons, std::string& str);
std::string prettifyUserFunction(Object* func);

// Macros and Typedefs

// what this is supposed to do: *function -> *function
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
extern bool LOG_TRAMPOLINE_TRACE;
extern bool LOG_STACK_TRACE;
extern bool LOG_PARSER;
extern bool LOG_MEMORY;
}  // namespace scm
