#include "operations.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <stack>
#include <string>
#include <vector>
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

std::vector<Object*> popN(std::stack<Object*>& stack, int n)
{
  if (stack.size() < n)
    throw(schemeException("stack doesn't contain " + std::to_string(n) + " arguments!"));
  std::vector<Object*> values;
  for (int i{0}; i < n; i++) {
    values.push_back(stack.top());
    stack.pop();
  }
  return values;
}

Object* addFunction(std::stack<Object*>& stack, int nArgs)
{
  int sum{0};
  auto arguments = popN(stack, nArgs);
  if (std::any_of(arguments.begin(), arguments.end(), isString)) {
    auto lambda = [](std::string a, Object* b) { return toString(b) + a; };
    std::string result = std::reduce(arguments.begin(), arguments.end(), std::string{}, lambda);
    return newString(result);
  }
  else if (std::any_of(arguments.begin(), arguments.end(), isFloatingPoint)) {
    auto lambda = [](double a, Object* b) { return getFloatValue(b) + a; };
    double result = std::reduce(arguments.begin(), arguments.end(), double(0.0), lambda);
    return newFloat(result);
  }
}
}  // namespace scm