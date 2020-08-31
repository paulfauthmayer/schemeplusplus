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

bool isFloat(std::string str)
{
  std::regex pattern(R"(^\-?[0-9]*\.[0-9]+$)");
  return std::regex_match(str, pattern);
}

bool isInt(std::string str)
{
  std::regex pattern(R"(^\-?[0-9]+$)");
  return std::regex_match(str, pattern);
}

bool isString(std::string str)
{
  std::regex pattern(R"(^\".*\"$)");
  return std::regex_match(str, pattern);
}

bool isSymbol(std::string str)
{
  std::regex pattern(R"(^[^\s^\d^.^\-^']\S*$|^\-$)");
  return std::regex_match(str, pattern);
}

// LEXING

std::vector<std::string> splitLine(std::string line)
{
  /**
   * The lexer is implemented on top of regular expressions!
   */
  std::vector<std::string> v;
  std::regex re(R"(\"[^\"]*\"|[#<>=\-\w\d\.\?\!]+|[\'\+\/\*\%\=\(\)])");
  for (std::sregex_iterator i = std::sregex_iterator(line.begin(), line.end(), re);
       i != std::sregex_iterator();
       i++) {
    v.push_back(std::smatch(*i).str());
  }
  return v;
}

Object* interpretList(std::vector<std::string>::iterator& current)
{
  Object *car, *cdr;
  if (*current == ")") {
    DLOG_IF_F(INFO, LOG_PARSER, "interpret %s as cons-end", (*current).c_str());
    return SCM_NIL;
  }
  car = interpretInput(current);
  cdr = interpretList(++current);
  return newCons(car, cdr);
}

Object* interpretInput(std::vector<std::string>::iterator& current)
{
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
   *
   * @param v Container of the currently detected tokens
   * @return boolean, is the input valid?
   */
  long openParanthesesCount{std::count(v.begin(), v.end(), "(")};
  long closeParanthesesCount{std::count(v.begin(), v.end(), ")")};
  return openParanthesesCount == closeParanthesesCount;
}

Object* readInput(std::istream* streamPtr, bool isFile)
{
  // setup container to keep the individual lexical elements
  std::vector<std::string> elements;
  std::string line;

  // read symbols until we have an evaluatable expression
  do {
    if (!std::getline(*streamPtr, line)) {
      DLOG_IF_F(INFO, LOG_PARSER, "EOF of input file detected");
      return SCM_EOF;
    }
    // TODO: input stack!
    line = line.substr(0, line.find(';'));
    std::vector<std::string> split = splitLine(line);
    elements.insert(elements.end(),
                    std::make_move_iterator(split.begin()),
                    std::make_move_iterator(split.end()));
    // wrap expression in parantheses for lazy typists
    // will f.ex. turn `+ 1 2 3` into `(+ 1 2 3)`
    if (elements.size() && isSymbol(elements[0]) && elements[0] != "(") {
      DLOG_IF_F(INFO, LOG_PARSER, "wrapping expression in parantheses");
      elements.insert(elements.begin(), "(");
      elements.push_back(")");
    }
  } while (!canBeEvaluated(elements) || elements.empty());

  std::vector<std::string>::iterator iter{elements.begin()};
  Object* obj{interpretInput(iter)};
  DLOG_IF_F(INFO, LOG_PARSER, "read expression %s", toString(obj).c_str());
  return obj;
}

}  // namespace scm
