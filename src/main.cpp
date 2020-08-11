#include <iostream>
#include "memory.hpp"
#include "reader.hpp"
#include "scheme.hpp"

#define DEBUG

int main(int argc, char** argv)
{
  // setup initial starting point
  initializeSingletons();
  std::cout << "scheme interpreter version " << 0.1 << '\n';

  // start the REPL
  scm::Object val;
  while (true) {
    // READ
    InputVector input = readInput();
    // EVALUATE
    val;
    // PRINT
  }
  // LOOP!

  return 0;
}