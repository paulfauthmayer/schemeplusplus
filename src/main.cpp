#include <iostream>
#include "reader.hpp"
#include "scheme.hpp"

#define DEBUG

int main(int argc, char** argv)
{
  std::cout << "scheme interpreter version " << 0.1 << '\n';

  while (true) {
    readInput();
  }

  return 0;
}