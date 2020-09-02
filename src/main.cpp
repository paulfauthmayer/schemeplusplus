#include <exception>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include "environment.hpp"
#include "evaluate.hpp"
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "repl.hpp"
#include "scheme.hpp"
#include "setup.hpp"
#include "test.hpp"

#define DEBUG

int main(int argc, char** argv)
{
  // intialise loguru
  loguru::init(argc, argv);

  // setup initial starting point
  scm::initializeSingletons();
  scm::Environment topLevelEnv{};
  scm::setupEnvironment(topLevelEnv);
  std::cout << "scheme interpreter version " << 0.1 << '\n';

  // run function setup for those written in scheme
  std::ifstream functionDefinitionStream;
  functionDefinitionStream.open("/Users/paul/repos/uni/dipl/src/std.scm");
  scm::repl(topLevelEnv, reinterpret_cast<std::istream*>(&functionDefinitionStream), true);

  // run unit tests, will crash if any tests fail!
  scm::runTests(topLevelEnv);

  // define input stream either as cin or from file
  std::istream* streamPtr;
  std::ifstream inputStream;
  bool isFile;
  switch (argc) {
    // just use the standard input!
    case 1: {
      DLOG_F(INFO, "using user input");
      isFile = false;
      streamPtr = &std::cin;
      break;
    }

    // stream from a .scm file
    case 2: {
      DLOG_F(INFO, "parsing input file %s", argv[1]);
      isFile = true;
      inputStream.open(argv[1]);
      if (!inputStream)
        return 1;
      streamPtr = &inputStream;
      break;
    }
    default:
      std::cout << "Too many arguments!";
      return 1;
      break;
  }

  // start the REPL
  if (!isFile)
    scm::printWelcome();
  scm::repl(topLevelEnv, streamPtr, isFile);

  return 0;
}
