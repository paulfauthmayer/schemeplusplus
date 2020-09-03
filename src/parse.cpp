#include "parse.hpp"
#include <iostream>
#include <loguru.hpp>
#include <regex>
#include <string>
#include <typeinfo>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

// CHECK FUNCTIONS

/**
 * Checks if a lexical element can be interpreted as a float value
 * @param str the lexical element
 * @returns can the element be interpreted as a float value?
 */
bool isFloat(std::string str)
{
  std::regex pattern(R"(^\-?[0-9]*\.[0-9]+$)");
  return std::regex_match(str, pattern);
}

/**
 * Checks if a lexical element can be interpreted as a integer value
 * @param str the lexical element
 * @returns can the element be interpreted as a integer value?
 */
bool isInt(std::string str)
{
  std::regex pattern(R"(^\-?[0-9]+$)");
  return std::regex_match(str, pattern);
}

/**
 * Checks if a lexical element can be interpreted as a string
 * @param str the lexical element
 * @returns can the element be interpreted as a string?
 */
bool isString(std::string str)
{
  std::regex pattern(R"(^\".*\"$)");
  return std::regex_match(str, pattern);
}

/**
 * Checks if a lexical element can be interpreted as a symbol
 * @param str the lexical element
 * @returns can the element be interpreted as a symbol?
 */
bool isSymbol(std::string str)
{
  std::regex pattern(R"(^[^\s^\d^.^\-^']\S*$|^\-$)");
  return std::regex_match(str, pattern);
}

// LEXING

/**
 * Splits an expression string into its individual lexical elements
 * @param line the expression line to split
 * @returns a vector of the resulting elements
 */
std::vector<std::string> splitLine(std::string line)
{
  std::vector<std::string> v;
  // 1st group: strings
  std::string stringRegex = R"(\"[^\"]*\")";
  // 2nd group: symbols and numbers
  std::string symbolsAndNumbersRegex = R"([#<>=\-\w\d\.\?\!]+)";
  // 3rd group: single character elements
  std::string singleCharacterRegex = R"([\'\+\/\*\%\=\(\)])";
  // combine all three groups into a single regular expression
  // this regex does all the splitting!
  std::regex re(stringRegex + "|" + symbolsAndNumbersRegex + "|" + singleCharacterRegex);
  for (std::sregex_iterator i = std::sregex_iterator(line.begin(), line.end(), re);
       i != std::sregex_iterator();
       i++) {
    v.push_back(std::smatch(*i).str());
  }
  return v;
}

/**
 * Interpret an element of a lexical element vector as a cons Object
 * @param current the iterator pointing to the current object in a vector
 * @returns the interpreted cons Object
 */
Object* interpretList(std::vector<std::string>::iterator& current)
{
  Object *car, *cdr;
  // ')' marks the end of the cons
  if (*current == ")") {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as cons-end", (*current).c_str());
    return SCM_NIL;
  }
  // car = current element, cdr = remaining elements
  car = interpretInput(current);
  cdr = interpretList(++current);
  return newCons(car, cdr);
}

/**
 * Interpret an element of a lexical element vector as a cons Object
 * @param current the iterator pointing to the current object in a vector
 * @returns the interpreted Object
 */
Object* interpretInput(std::vector<std::string>::iterator& current)
{
  // check as what type of object the element can be interpreted
  if (isInt(*current)) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as integer", (*current).c_str());
    return newInteger(std::stoi(*current));
  }
  if (isFloat(*current)) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as float", (*current).c_str());
    return newFloat(stof(*current));
  }
  else if (isString(*current)) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as string", (*current).c_str());
    return newString((*current).substr(1, (*current).length() - 2));
  }
  else if (*current == std::string("#t")) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as boolean", (*current).c_str());
    return SCM_TRUE;
  }
  else if (*current == std::string("#f")) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as boolean", (*current).c_str());
    return SCM_FALSE;
  }
  else if (*current == "(") {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as cons", (*current).c_str());
    return interpretList(++current);
  }
  else if (*current == "'") {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as quote", (*current).c_str());
    Object* quoteContents{interpretInput(++current)};
    Object* cdr = (quoteContents == SCM_NIL) ? SCM_NIL : newCons(quoteContents, SCM_NIL);
    return newCons(newSymbol("quote"), cdr);
  }
  else if (*current == "exit!") {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as EOF", (*current).c_str());
    return SCM_EOF;
  }
  else if (isSymbol(*current)) {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as symbol", (*current).c_str());
    return newSymbol(*current);
  }
  else {
    schemeThrow("{{" + *current + "}} could not be interpreted.");
  }
}

bool canBeEvaluated(const std::vector<std::string>& v)
{
  /**
   * Check whether the input so far can be evaluated.
   * @param v Container of the currently detected tokens
   * @returns boolean, is the input valid?
   */
  long openParanthesesCount{std::count(v.begin(), v.end(), "(")};
  long closeParanthesesCount{std::count(v.begin(), v.end(), ")")};
  return openParanthesesCount == closeParanthesesCount;
}

/**
 * Read expressions from an input stream. Will continue to read until a valid expression has been
 * detected.
 * @param streamPtr a pointer pointing to the stream object from which to read
 * @param isFile if true, return on EOF
 * @returns the read Object
 */
Object* readInput(std::istream* streamPtr, bool isFile)
{
  // setup container to keep the individual lexical elements
  std::vector<std::string> elements;
  std::string line;
  int emptyCount{0};

  // read symbols until we have an evaluatable expression
  do {
    if (!std::getline(*streamPtr, line)) {
      DLOG_IF_F(INFO, LOG_PARSER, "EOF of input file detected");
      return SCM_EOF;
    }
    // if the user enters three newlines in succession, return to repl
    if (line.size() == 0 && !isFile) {
      if (++emptyCount > 2) {
        DLOG_IF_F(WARNING, LOG_PARSER, "user cancelled input, return to loop");
        return SCM_VOID;
      }
    }
    else {
      emptyCount = 0;
    }

    // remove comments, as they shouldn't be interpreted
    line = line.substr(0, line.find(';'));

    // split the read line into lexical elements and store them
    std::vector<std::string> split = splitLine(line);
    elements.insert(elements.end(),
                    std::make_move_iterator(split.begin()),
                    std::make_move_iterator(split.end()));

    // wrap expression in parantheses for lazy typists
    // will f.ex. turn `+ 1 2 3` into `(+ 1 2 3)`
    // but f.ex. not `-1` into `(-1)`
    if (elements.size() && isSymbol(elements[0]) && elements[0] != "(") {
      DLOG_IF_F(INFO, LOG_PARSER, "wrapping expression in parantheses");
      elements.insert(elements.begin(), "(");
      elements.push_back(")");
    }
    // repeat until we have an interpretable sequence of elements
  } while (!canBeEvaluated(elements) || elements.empty());

  // interpret the detected elements and return for evaluation
  std::vector<std::string>::iterator iter{elements.begin()};
  Object* obj{interpretInput(iter)};
  DLOG_IF_F(INFO, LOG_PARSER, "read expression %s", toString(obj).c_str());
  return obj;
}

}  // namespace scm
