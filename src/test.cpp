#include "test.hpp"
#include <math.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include "evaluate.hpp"
#include "memory.hpp"
#include "operations.hpp"
#include "parse.hpp"
#include "repl.hpp"
#include "scheme.hpp"
#include "setup.hpp"

namespace scm {

// Environment used for this testing
static Environment testEnv{};

// helpers

/**
 * Checks whether the two values can be treated as equal.
 * Used to get around floating point errors during testin.
 * The values are treated equal if they're in a certain epsilon to one another.
 * IMPORTANT: never ever ever use this outside of a scenario you can't control.
 * It's fine during testing but seriously, don't use this!
 * @param a the first value
 * @param b the second value
 * @returns whether the values are similar enough to each other in order to be treated equal
 */
template <typename T>
bool equalFloatValue(T a, T b)
{
  return fabs(a - b) < 0.001;
}

/**
 * Evaluate an input string in the test environment.
 * @param inputString the string to be evaluated
 * @returns the result of the evaluation, EOF on fail
 */
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
  return SCM_EOF;
}

/**
 * Test a given expression, compare the result against the
 * expected result and log the conclusion.
 * @param inputString the expression(s) to be evaluated
 * @param expectedOutput the expected output of the expression
 * @param message a message describing the purpose of the test
 */
void testExpression(const std::string& inputString,
                    const std::string& expectedOutput,
                    const std::string message)
{
  Object* result = evaluateString(inputString);
  std::string strResult{toString(result)};
  // cut off quotation marks
  if (hasTag(result, TAG_STRING)) {
    strResult = strResult.substr(1, strResult.length() - 2);
  }
  bool correctResult{strResult == expectedOutput};
  if (correctResult) {
    DLOG_IF_F(INFO, LOG_TESTS, "%s", message.c_str());
  }
  else {
    LOG_F(ERROR,
          "%s | expected: %s | got: %s",
          message.c_str(),
          expectedOutput.c_str(),
          strResult.c_str());
  }
}

/** @overload */
void testExpression(const std::string& inputString, int expectedOutput, std::string message)
{
  Object* result = evaluateString(inputString);
  bool correctResult{getIntValue(result) == expectedOutput};
  if (correctResult) {
    DLOG_IF_F(INFO, LOG_TESTS, "%s", message.c_str());
  }
  else {
    LOG_F(
        ERROR, "%s | expected: %d | got: %d", message.c_str(), expectedOutput, getIntValue(result));
  }
}

/** @overload */
void testExpression(const std::string& inputString, double expectedOutput, std::string message)
{
  Object* result = evaluateString(inputString);
  bool correctResult{equalFloatValue((getFloatValue(result)), expectedOutput)};
  if (correctResult) {
    DLOG_IF_F(INFO, LOG_TESTS, "%s", message.c_str());
  }
  else {
    LOG_F(ERROR,
          "%s | expected: %f | got: %f",
          message.c_str(),
          expectedOutput,
          getFloatValue(result));
  }
}

/** @overload */
void testExpression(const std::string& inputString, Object* expectedOutput, std::string message)
{
  Object* result = evaluateString(inputString);
  bool correctResult{result == expectedOutput};
  if (correctResult) {
    DLOG_IF_F(INFO, LOG_TESTS, "%s", message.c_str());
  }
  else {
    LOG_F(ERROR,
          "%s | expected: %s | got: %s",
          message.c_str(),
          toString(expectedOutput).c_str(),
          toString(result).c_str());
  }
}

/**
 * Run a number of tests to check whether everything works as expected.
 * @param env An environment to test in, will create a copy in order not to change anything in the
 * original. All functions and syntax needs to be setup in order for these tests to work.
 */
void runTests(const Environment& env)
{
  // setup environment for testing
  testEnv = env;

  // parsing
  testExpression("15", 15, "test | parser: integer");
  testExpression("-15", -15, "test | parser: negative integer");
  testExpression("1.5", 1.5, "test | parser: float");
  testExpression(".5", 0.5, "test | parser: dotted float");
  testExpression("-1.5", -1.5, "test | parser: negative float");
  testExpression("'()", SCM_NIL, "test | parser: nil");
  testExpression("+ 1 2 3", 6, "test | parser: wrap in parantheses");

  // memory

  // evaluation

  /// syntax

  // begin
  testExpression("(begin (+ 1 1) (+ 2 2))", 4, "test | syntax: begin");

  // define and lambdas
  evaluateString("(define a 10)");
  testExpression("a", 10, "test | syntax: define");

  evaluateString("(define plus1 (lambda (x) (+ x 1)))");
  testExpression("(plus1 1)", 2, "test | syntax: define lambdas");

  evaluateString("(define (minus1 x) (- x 1))");
  testExpression("(minus1 3)", 2, "test | syntax: define shorthand lambdas");

  // quote
  testExpression("(quote 1)", 1, "test | syntax: quote single value");
  testExpression("(quote 1 2 3)", 1, "test | syntax: quote first value");
  testExpression("(quote (1 2 3))", "( 1 2 3 )", "test | syntax: quote list");
  testExpression("'(1 2 3)", "( 1 2 3 )", "test | syntax: shorthand quote list");

  // if
  testExpression("(if #t 1 2)", 1, "test | syntax: if true");
  testExpression("(if #f 1 2)", 2, "test | syntax: if false");
  testExpression("(if (> 2 4) 1 2)", 2, "test | syntax: if expression");

  // set!
  testExpression("(begin (set! a 10) (a))", 10, "test | syntax: set");
  testExpression(
      "(begin \
  (define (a x) (set! set-example2 10) (+ x 2))\
  (a 2)\
  (set-example2))\
  ",
      10,
      "test | syntax: set in function");

  // addition
  testExpression("(+ 1 2)", 3, "test | func: integer additon");
  testExpression("(+ 1.1 2)", 3.1, "test | func: mixed additon");
  testExpression("(+ 1.1 2.2)", 3.3, "test | func: float additon");
  testExpression("(+ \"hello \" \"world!\")", "hello world!", "test | func: string additon");
  testExpression(
      "(+ \"hello \" 1 \" world!\")", "hello 1 world!", "test | func: mixed string additon");

  // subtraction
  testExpression("(- 1 2)", -1, "test | func: integer subtraction");
  testExpression("(- 1 2.5)", -1.5, "test | func: float subtraction");
  testExpression("(- 1)", -1, "test | func: negate integer");

  // division
  testExpression("(/ 4 2)", 2.0, "test | func: integer division");
  testExpression("(/ 5 2)", 2.5, "test | func: integer division with float result");
  testExpression("(/ 5 2.5)", 2.0, "test | func: mixed division");

  // modulo
  testExpression("(% 5 2.5)", 0.0, "test | func: module zero remainder");
  testExpression("(% 5 1.5)", 0.5, "test | func: module non-zero remainder");

  // comparisons
  testExpression("(> 4 2)", SCM_TRUE, "test | func: greater than true");
  testExpression("(> 2 2)", SCM_FALSE, "test | func: greater than false");
  testExpression("(< 1 2)", SCM_TRUE, "test | func: lesser than true");
  testExpression("(< 4 2)", SCM_FALSE, "test | func: lesser than false");
  testExpression("(= 2 2)", SCM_TRUE, "test | func: equal number integer");
  testExpression("(= 2.0 2.0)", SCM_TRUE, "test | func: equal number float");
  testExpression("(= 2.0 2)", SCM_TRUE, "test | func: equal number mixed");
  testExpression("(eq? nil nil)", SCM_TRUE, "test | func: eq? true");
  testExpression("(eq? 1 1)", SCM_FALSE, "test | func: eq? false");
  testExpression("(equal? 1 1)", SCM_TRUE, "test | func: equal? integer true");
  testExpression("(equal? 1.0 1.0)", SCM_TRUE, "test | func: equal? float true");
  testExpression("(equal? 1.0 1)", SCM_TRUE, "test | func: equal? mixed true");
  testExpression("(equal? \"asdf\" \"asdf\")", SCM_TRUE, "test | func: equal? string true");
  testExpression("(equal? 1 10)", SCM_FALSE, "test | func: equal? integer false");
  testExpression("(equal? 8.0 1.0)", SCM_FALSE, "test | func: equal? float false");
  testExpression("(equal? 1.2 1)", SCM_FALSE, "test | func: equal? mixed false");
  testExpression("(equal? \"hello!\" \"asdf\")", SCM_FALSE, "test | func: equal? string false");

  // type checks
  testExpression("(number? 42)", SCM_TRUE, "test | func: is number true");
  testExpression("(number? 42.0)", SCM_TRUE, "test | func: is number true float");
  testExpression("(number? #t)", SCM_FALSE, "test | func: is number false");
  testExpression("(string? \"42\")", SCM_TRUE, "test | func: is string true");
  testExpression("(string? #t)", SCM_FALSE, "test | func: is string false");
  testExpression("(cons? '(1 2 3))", SCM_TRUE, "test | func: is cons true");
  testExpression("(cons? 1)", SCM_FALSE, "test | func: is cons false");

  // TODO: to be continued ...
}

}  // namespace scm