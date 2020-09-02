#include "setup.hpp"
#include <iostream>
#include "environment.hpp"
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

/**
 * Define a new builtin syntax object in an environment
 * @param env the environment in which to define the syntax
 * @param name the name of the syntax
 * @param nArgs the number of arguments required by the syntax, specify -1 for 0-inf arguments
 * @param tag the tag of the function for identification purposes
 * @param helpText a text that's shown when help is requested for this syntax
 */
void defineNewSyntax(Environment& env,
                     std::string name,
                     int nArgs,
                     FunctionTag tag,
                     std::string helpText)
{
  Object* func{newSyntax(name, nArgs, tag, helpText)};
  define(env, newSymbol(name), func);
}

/**
 * Define a new builtin function object in an environment
 * @param env the environment in which to define the function
 * @param name the name of the function
 * @param nArgs the number of arguments required by the function, specify -1 for 0-inf arguments
 * @param tag the tag of the function for identification purposes
 * @param helpText a text that's shown when help is requested for this function
 */
void defineNewBuiltinFunction(Environment& env,
                              std::string name,
                              int nArgs,
                              FunctionTag tag,
                              std::string helpText)
{
  Object* func{newBuiltinFunction(name, nArgs, tag, helpText)};
  define(env, newSymbol(name), func);
}

/**
 * Setup an environment with all builtin functions and syntax.
 * @param env the environment in which to define the operations
 */
void setupEnvironment(Environment& env)
{
  std::string helpText{};
  // setup syntax
  helpText =
      "returns the first argument:\n\
  (quote 1 2 3) -> 1\n\
  (quote (1 2 3)) -> (1 2 3)\n\
  shorthand: '(1 2 3)";
  defineNewSyntax(env, "quote", -1, SYNTAX_QUOTE, helpText);
  helpText =
      "returns the first expression if the condition is true, the second otherwise:\n\
  (if #t 1 2) -> 1\n\
  (if #f 1 2) -> 2";
  defineNewSyntax(env, "if", -1, SYNTAX_IF, helpText);
  helpText =
      "defines a value to the given variable name\n\
  (define a 10) -> a := 10\n\
  can be used in shorthand form for lambda definition\n\
  (define (plus1 x) (+ x 1)";
  defineNewSyntax(env, "define", -1, SYNTAX_DEFINE, helpText);
  helpText =
      "defines a value to the given variable name in all environments\n\
  (set! a 10) -> a := 10";
  defineNewSyntax(env, "set!", -1, SYNTAX_SET, helpText);
  helpText =
      "defines a new function\n\
  (lambda (arg1 arg2) (+ arg1 arg2)\n\
  use in combination with define";
  defineNewSyntax(env, "lambda", -1, SYNTAX_LAMBDA, helpText);
  helpText =
      "evaluate multiple expressions and return last result\n\
  (begin (+ 1 1) (+ 2 2)) -> 4";
  defineNewSyntax(env, "begin", -1, SYNTAX_BEGIN, helpText);
  helpText =
      "show help text for a given element\n\
  help -> shows all defined variables, functions and syntax elements\n\
  (help fname) -> shows a help text for the given function";
  defineNewSyntax(env, "help", 0, SYNTAX_HELP, helpText);

  // setup builtin functions
  helpText =
      "adds multiple numbers and/or strings\n\
  (+ 1 2 3) -> 6\n\
  (+ 1 2 2.5) -> 5.5\n\
  (+ \"hello \" \"world!\") -> \"hello world!\"\n\
  (+ \"hello \" 1 \" world!\") -> \"hello 1 world!\"\n\
  priority: string > float > integer";
  defineNewBuiltinFunction(env, "+", -1, FUNC_ADD, helpText);
  helpText =
      "subtracts the sum of multiple numbers from the first argument\n\
  (- 1 2 3) -> 4\n\
  (- 1 2 2.5) -> 3.5";
  defineNewBuiltinFunction(env, "-", -1, FUNC_SUB, helpText);
  helpText =
      "multiplies all arguments with each other\n\
  (* 2 2 2) -> 8";
  defineNewBuiltinFunction(env, "*", -1, FUNC_MULT, helpText);
  helpText =
      "divides the first argument by the product of all other arguments\n\
  (/ 2 2 2) -> 0.5";
  defineNewBuiltinFunction(env, "/", -1, FUNC_DIV, helpText);
  helpText =
      "returns true if same exact object\n\
  (eq? 1 1) -> #f\n\
  (eq? a a) -> #t";
  defineNewBuiltinFunction(env, "eq?", 2, FUNC_EQ, helpText);
  helpText =
      "returns true if the passed strings are identical\n\
  (equal-string? \"asd\" \"asd\") -> #t\n\
  (equal-string? \"asd\" \"qwe\") -> #f";
  defineNewBuiltinFunction(env, "equal-string?", 2, FUNC_EQUAL_STRING, helpText);
  helpText =
      "returns true if the passed numbers are equal\n\
  (= 1 1) -> #t\n\
  (= 1 2) -> #f\n\
  (= 1 1.0) -> #t";
  defineNewBuiltinFunction(env, "=", 2, FUNC_EQUAL_NUMBER, helpText);
  helpText =
      "returns true if the first numbers is greater than the latter\n\
  (> 1 1) -> #f\n\
  (> 3 2) -> #t\n\
  (> 0 1.0) -> #f";
  defineNewBuiltinFunction(env, ">", 2, FUNC_GT, helpText);
  helpText =
      "returns true if the first numbers is lesser than the latter\n\
  (< 1 1) -> #f\n\
  (< 3 2) -> #f\n\
  (< 0 1.0) -> #t";
  defineNewBuiltinFunction(env, "<", 2, FUNC_LT, helpText);
  helpText =
      "assembles a new cons object\n\
  (cons 1 2) -> (1 . 2)\n\
  (cons 1 '(2 3)) -> (1 2 3)";
  defineNewBuiltinFunction(env, "cons", 2, FUNC_CONS, helpText);
  helpText =
      "get the car of a cons\n\
  (car '(1 2 3)) -> 1";
  defineNewBuiltinFunction(env, "car", 1, FUNC_CAR, helpText);
  helpText =
      "get the cdr of a cons\n\
  (cdr '(1 2 3)) -> (2 3)";
  defineNewBuiltinFunction(env, "cdr", 1, FUNC_CDR, helpText);
  helpText =
      "assembles a list with the given arguments\n\
  (list 1 2 3) -> (1 2 3)";
  defineNewBuiltinFunction(env, "list", -1, FUNC_LIST, helpText);
  helpText =
      "displays the passed argument, best used within functions\n\
  (display 1 2 3) -> returns void, prints (1 2 3)";
  defineNewBuiltinFunction(env, "display", -1, FUNC_DISPLAY, helpText);
  helpText = "returns the function body of a given lambda";
  defineNewBuiltinFunction(env, "function-body", 1, FUNC_FUNCTION_BODY, helpText);
  helpText = "returns the function argument list of a given lambda";
  defineNewBuiltinFunction(env, "function-arglist", 1, FUNC_FUNCTION_ARGLIST, helpText);
  helpText = "returns true if the argument is a string";
  defineNewBuiltinFunction(env, "string?", 1, FUNC_IS_STRING, helpText);
  helpText = "returns true if the argument is an integer or a float value";
  defineNewBuiltinFunction(env, "number?", 1, FUNC_IS_NUMBER, helpText);
  helpText = "returns true if the argument is a cons object";
  defineNewBuiltinFunction(env, "cons?", 1, FUNC_IS_CONS, helpText);
  helpText = "returns true if the argument is a builtin function";
  defineNewBuiltinFunction(env, "function?", 1, FUNC_IS_FUNC, helpText);
  helpText = "returns true if the argument is a user defined function";
  defineNewBuiltinFunction(env, "user-function?", 1, FUNC_IS_USERFUNC, helpText);
  helpText = "returns true if the argument is real bool value";
  defineNewBuiltinFunction(env, "bool?", 1, FUNC_IS_BOOL, helpText);
}

}  // namespace scm