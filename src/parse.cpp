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
  for (std::sregex_iterator i = std::sregex_iterator(line.begin(), line.end(), re); i != std::sregex_iterator(); i++) {
    v.push_back(std::smatch(*i).str());
  }
  return v;
}

void tokenizeLine(std::vector<std::string> lineVector, InputVector& tokens)
{
  /**
   * Creates tokens out of the elements detected in an input.
   *
   * @param lineVector contains the individual strings that make up the input
   */
  for (std::string item : lineVector) {
    if (isString(item)) {
      std::cout << "read string";
      tokens.push_back(newString(item));
    }
    else if (isInt(item)) {
      std::cout << "read integer";
      tokens.push_back(newInteger(std::stoi(item)));
    }
    else if (isFloat(item)) {
      std::cout << "read float";
      tokens.push_back(newFloat(stof(item)));
    }
    else if (isSymbol(item)) {
      std::cout << "read symbol";
      tokens.push_back(newSybmol(item));
    }
    else {
      std::cout << "Invalid symbol encountered!";
    }
    std::cout << ": " << item << "\n";
  }
}

bool canBeEvaluated(const InputVector& v)
{
  /**
   * Check whether the input so far can be evaluated.
   *
   * @param v Container of the currently detected tokens
   * @return boolean, is the input valid?
   */
  int openParanthesesCount{};
  int closeParanthesesCount{};
  return openParanthesesCount == closeParanthesesCount;
}

InputVector readInput()
{
  // setup container to keep tokens
  InputVector v;
  std::string line;

  // read symbols until we have an evaluatable expression
  do {
    std::cout << "> ";
    std::getline(std::cin, line);
    std::cout << "read: " << line << '\n';
    // TODO: input stack!
    std::vector split = splitLine(line);
    for (auto s : split)
      std::cout << "\t- " << s << "\n";
    tokenizeLine(split, v);
    std::cout << "successfully read line!\n";
  } while (!canBeEvaluated(v));

  return v;
}