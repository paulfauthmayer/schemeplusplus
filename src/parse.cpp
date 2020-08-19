#include "parse.hpp"
#include <iostream>
#include <regex>
#include <string>
#include <typeinfo>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

// CHECK FUNCTIONS

bool isFloat(std::string str)
{
  std::regex pattern(R"(^[0-9]+\.[0-9]*$)");
  return std::regex_match(str, pattern);
}

bool isInt(std::string str)
{
  std::regex pattern(R"(^[0-9]+$)");
  return std::regex_match(str, pattern);
}

bool isString(std::string str)
{
  std::regex pattern(R"(^\".*\"$)");
  return std::regex_match(str, pattern);
}

bool isSymbol(std::string str)
{
  std::regex pattern(R"(^[^\s^\d]\S*$)");
  return std::regex_match(str, pattern);
}

// LEXING

std::vector<std::string> splitLine(std::string line)
{
  /**
   * The lexer is implemented on top of regular expressions!
   */
  std::vector<std::string> v;
  std::regex re(R"(\".*\"|[\+\/\*\-\%\(\)]|[\w\d]+)");
  for (std::sregex_iterator i = std::sregex_iterator(line.begin(), line.end(), re);
       i != std::sregex_iterator();
       i++) {
    v.push_back(std::smatch(*i).str());
  }
  return v;
}

scm::Object* interpretList(std::vector<std::string>::iterator& current)
{
  std::cout << "car: " << *current << " cdr: " << *(current + 1) << "\n";
  scm::Object *car, *cdr;

  car = interpretInput(current);
  cdr = (*(++current) == ")") ? SCM_NIL : interpretList(current);

  return newCons(car, cdr);
}

scm::Object* interpretInput(std::vector<std::string>::iterator& current)
{
  if (isInt(*current))
    return newInteger(std::stoi(*current));
  if (isFloat(*current))
    return newFloat(stof(*current));
  else if (isString(*current))
    return newString(*current);
  else if (*current == "(")
    return interpretList(++current);
  else if (isSymbol(*current))
    return newSybmol(*current);
  else
    throw("{{" + *current + "}} could not be interpreted.");
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
  std::cout << "[open: " << openParanthesesCount << ", close: " << closeParanthesesCount << "]\n";
  return openParanthesesCount == closeParanthesesCount;
}

scm::Object* readInput(std::istream* streamPtr)
{
  // setup container to keep the individual lexical elements
  std::vector<std::string> elements;
  std::string line;

  // read symbols until we have an evaluatable expression
  std::cout << "> ";
  do {
    std::getline(*streamPtr, line);
    // TODO: input stack!
    // TODO: handle fileStream exit condition
    std::vector<std::string> split = splitLine(line);
    elements.insert(elements.end(),
                    std::make_move_iterator(split.begin()),
                    std::make_move_iterator(split.end()));
  } while (!canBeEvaluated(elements));

  std::vector<std::string>::iterator iter{elements.begin()};
  scm::Object* obj{interpretInput(iter)};
  std::cout << "successfully read expression! " << scm::toString(obj) << "\n";
  return obj;
}
