#include "reader.hpp"
#include <iostream>
#include <regex>
#include <vector>
#include "scheme.hpp"

void cleanWhiteSpace()
{
  char peeked{static_cast<char>(std::cin.peek())};
  if (isspace(peeked)) {
    std::cin.ignore(1);
    cleanWhiteSpace();
  }
  return;
}

bool isFloat(std::string str)
{
  std::regex regex_pattern("[0-9]+.[0-9]*");
  return std::regex_match(str, regex_pattern);
}

bool isInt(std::string str)
{
  std::regex regex_pattern("?-[0-9]+");
  return std::regex_match(str, regex_pattern);
}

bool isString(std::string str)
{
  std::regex regex_pattern(".*\"$");
  return std::regex_match(str, regex_pattern);
}

std::vector<std::string> splitLine(std::string line)
{
  std::vector<std::string> v;
  std::regex regex_split("(\")?(?(1)(.*\")|([\d\w]*|[\+\/\*\(\)]))\s*");

  return v;
}

void tokenizeLine(std::string line, InputVector& v)
{
  std::string next;
  if (next.at(0) == '"') {
    // first stitch together the relevant part
    do {
      std::string cont;
      next.append(' ' + cont);
    } while (next.back() != '"');
  }
  else if (isFloat(next)) {
  }
}

bool validInput(InputVector v)
{
  return false;
}

InputVector readInput()
{
  InputVector v;
  std::string line;

  std::cout << "> ";

  while (std::getline(std::cin, line)) {
    // TODO: input stack!
    std::cout << line << '\n';

    std::cout << "> ";
  }
}