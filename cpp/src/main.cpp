#include <iostream>
#include "eval.hpp"
#include "printer.hpp"
#include "reader.hpp"
#include "scheme.hpp"

#define DEBUG

int main(int argc, char** argv)
{
#ifdef DEBUG
  std::cout << argc << " arguments were passed\n";
  for (int i{0}; i < argc; i++) {
    std::cout << "arg" << i << ": " << argv[i] << '\n';
  }
#endif

  while (true) {
    // read and interpret the next expression
    scmObject expr{scm_read()};
    std::cout << "read returned object with tag " << expr->tag << '\n';
    scmObject val{scm_eval(expr)};
    scm_print(val);
    std::cout << '\n';
  }

  return 0;
}