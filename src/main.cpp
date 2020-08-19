#include <fstream>
#include <iostream>
#include "evaluate.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "scheme.hpp"

#define DEBUG

scm::Object* repl(){};

int main(int argc, char** argv)
{
  // setup initial starting point
  initializeSingletons();
  std::cout << "scheme interpreter version " << 0.1 << '\n';

  // define input stream either as cin or from file
  std::istream* streamPtr;
  std::ifstream inputStream;
  std::streambuf* orig_cin = 0;
  switch (argc) {
    case 1:
      // just use the standard input!
      std::cout << "using user input\n";
      streamPtr = &std::cin;
      break;

    case 2:
      // redirect streambuffer of std::cin to the input file!
      std::cout << "using file " << argv[1] << " as input\n";
      inputStream.open(argv[1]);
      if (!inputStream)
        return 1;
      streamPtr = &inputStream;
      break;

    default:
      std::cout << "Too many arguments!";
      break;
  }

  // start the REPL
  do {
    try {
      // READ
      std::cout << " // READ // \n";
      scm::Object* expression = readInput(streamPtr);
      // EVALUATE
      std::cout << " // EVALUATE // \n";
      scm::Object* value = evaluate(expression);
      // PRINT
      std::cout << " // PRINT // \n";
      std::cout << scm::toString(value) << std::endl;
    }
    catch (const char* exception) {
      std::cerr << "[ERROR] " << exception;
    }
  } while (std::cin);  // LOOP!

  return 0;
}
