#include "test.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
#include "operations_trampoline.hpp"
#include "parse.hpp"
#include "repl.hpp"
#include "scheme.hpp"
#include "setup.hpp"

namespace scm {

// Environment used for this testing
static Environment testEnv{};

// helpers
Object* evaluateString(const std::string& inputString)
{
  try {
    std::stringstream ss = std::stringstream(inputString);
    Object* expression = readInput(&ss, true);
    Object* value = trampoline::evaluateExpression(testEnv, expression);
    return value;
  }
  catch (const schemeException& e) {
    std::cerr << e.what() << '\n';
  }
  return NULL;
}

void testExpression(const std::string& inputString,
                    const std::string& expectedOutput,
                    std::string message)
{
  Object* result = evaluateString(inputString);
  assert(toString(result) == expectedOutput && message.c_str());
}

void testExpression(const std::string& inputString, int expectedOutput, std::string message)
{
  Object* result = evaluateString(inputString);
  assert(getIntValue(result) == expectedOutput && message.c_str());
}

void testExpression(const std::string& inputString, double expectedOutput, std::string message)
{
  Object* result = evaluateString(inputString);
  assert(getFloatValue(result) == expectedOutput && message.c_str());
}

void runTests(const Environment& env)
{
  // setup environment for testing
  testEnv = env;
  Object* result;
  std::string inputString{"(help)"};
  std::string varName = "a";
  int expected_int;
  double expected_double;
  std::string expected_string;

  // parsing
  inputString = "1";
  result = evaluateString(inputString);
  assert(hasTag(result, TAG_INT) && "did not correctly read integer");

  inputString = "-1";
  result = evaluateString(inputString);
  assert(getIntValue(result) == -1 && "did not correctly read negative interger");

  inputString = "1.5";
  result = evaluateString(inputString);
  assert(hasTag(result, TAG_FLOAT) && "did not correctly read float");

  inputString = ".5";
  result = evaluateString(inputString);
  assert(hasTag(result, TAG_FLOAT) && "did not correctly read . float");

  inputString = "1.5";
  result = evaluateString(inputString);
  assert(hasTag(result, TAG_FLOAT) && "did not correctly read negative float");

  inputString = "1.5";
  result = evaluateString(inputString);
  assert(getFloatValue(result) == 1.5 && "did not correctly read float value");

  inputString = "'()";
  result = evaluateString(inputString);
  assert(result == SCM_NIL && "parser: nil");

  inputString = "'(1 2 3)";
  result = evaluateString(inputString);
  assert(toString(result) == "( 1 2 3 )" && "parser: quoted list");

  // memory

  /// evaluation

  // syntax

  // define and lambdas
  inputString = "(define a 10)";
  result = evaluateString(inputString);
  varName = "a";
  assert(getIntValue(getVariable(testEnv, varName)) == 10 && "syntax: define variables");

  inputString = "(define a (lambda (x) (+ x 1)))";
  result = evaluateString(inputString);
  varName = "a";
  assert(hasTag(getVariable(testEnv, varName), TAG_FUNC_USER) && "syntax: define lambdas");

  inputString = "(define (b x) (+ x 1))";
  result = evaluateString(inputString);
  varName = "b";
  assert(hasTag(getVariable(testEnv, varName), TAG_FUNC_USER) && "syntax: define lambdas");

  // quote
  inputString = "(quote 1 2 3)";
  result = evaluateString(inputString);
  assert(toString(result) == "1" && "syntax: quote first arg");

  inputString = "(quote (1 2 3))";
  result = evaluateString(inputString);
  assert(toString(result) == "( 1 2 3 )" && "syntax: quote list");

  // if
  inputString = "(if #t 1 2)";
  result = evaluateString(inputString);
  assert(toString(result) == "1" && "syntax: if true");

  inputString = "(if #f 1 2)";
  result = evaluateString(inputString);
  assert(toString(result) == "2" && "syntax: if false");

  inputString = "(if (> 2 4) 1 2)";
  result = evaluateString(inputString);
  assert(toString(result) == "2" && "syntax: if expression");

  // begin
  inputString =
      "(begin \
  (+ 1 1)\
  (+ 2 2))";
  result = evaluateString(inputString);
  assert(toString(result) == "4" && "syntax: begin");

  // set!
  inputString = "(set! set-example1 10)";
  result = evaluateString(inputString);
  varName = "set-example1";
  assert(getIntValue(getVariable(testEnv, varName)) == 10 && "syntax: define variables");

  inputString = "(define (a x) (set! set-example2 10) (+ x 2))";
  result = evaluateString(inputString);
  inputString = "(a 2)";
  result = evaluateString(inputString);
  varName = "set-example2";
  assert(getVariable(testEnv, varName) != NULL && "syntax: set! variables");

  // addition
  inputString = "(+ 11 22)";
  result = evaluateString(inputString);
  assert(getIntValue(result) == 33 && "integer additon");

  inputString = "(+ 1.1 2.2)";
  result = evaluateString(inputString);
  assert(static_cast<int>(getFloatValue(result) * 10) == 33 && "float addition");

  inputString = "(+ \"hello \" \"world!\")";
  result = evaluateString(inputString);
  assert(getStringValue(result) == "hello world!" && "string addition");

  inputString = "(+ 1.1 2)";
  result = evaluateString(inputString);
  assert(static_cast<int>(getFloatValue(result) * 10) == 31 && "mixed numeric addition");

  inputString = "(+ \"hello \" 1 \" world!\")";
  result = evaluateString(inputString);
  assert(getStringValue(result) == "hello 1 world!" && "string addition");

  inputString = "(+ \"hello \" 1 \" world!\")";
  result = evaluateString(inputString);
  assert(getStringValue(result) == "hello 1 world!" && "string addition");

  // subtraction
  inputString = "(- 1 2)";
  result = evaluateString(inputString);
  assert(getIntValue(result) == -1 && "integer subtraction");

  inputString = "(- 1 2.5)";
  result = evaluateString(inputString);
  assert(static_cast<int>(getFloatValue(result) * 10) == -15 && "float subtraction");

  // division
  inputString = "(/ 4 2)";
  result = evaluateString(inputString);
  assert(getFloatValue(result) == 2 && "func: integer division");

  inputString = "(/ 5 2)";
  result = evaluateString(inputString);
  assert(getFloatValue(result) == 2.5 && "func: integer division with float result");

  inputString = "(/ 5 2.5)";
  result = evaluateString(inputString);
  assert(getFloatValue(result) == 2 && "func: mixed division");

  inputString = "(/ 5 2.5)";
  result = evaluateString(inputString);
  assert(getFloatValue(result) == 2.0 && "func: mixed division");

  // modulo
  inputString = "(% 5 2.5)";
  expected_double = 0;
  testExpression(inputString, expected_double, "func: 0 modulo");

  inputString = "(% 5 2)";
  expected_int = 1;
  testExpression(inputString, expected_int, "func: 0 modulo");

  // TODO: to be continued ...
}

}  // namespace scm