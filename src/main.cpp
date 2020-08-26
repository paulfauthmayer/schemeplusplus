#include <exception>
#include <fstream>
#include <iostream>
#include <loguru.hpp>
#include "environment.hpp"
#include "evaluate.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "scheme.hpp"
#include "setup.hpp"
#include "test.hpp"

#define DEBUG

void repl(scm::Environment& env, std::istream* streamPtr, bool isFile = true)
{
  do {
    try {
      // READ
      DLOG_F(INFO, "(R)epl");
      scm::Object* expression = scm::readInput(streamPtr, isFile);
      if (expression == scm::SCM_EOF) {
        return;
      }
      // EVALUATE
      DLOG_F(INFO, "r(E)pl");
      scm::Object* value = scm::evaluate(env, expression);
      // PRINT
      DLOG_F(INFO, "re(P)l");
      if (value != scm::SCM_VOID) {
        std::cout << "--> " << scm::toString(value) << std::endl;
      }
      DLOG_F(INFO, "rep(L)");
    }
    catch (scm::schemeException& e) {
      std::cerr << e.what() << '\n';
    }
    catch (std::exception& e) {
      std::cerr << "[CPP::ERROR] " << e.what() << '\n';
    }
  } while (true);  // LOOP!
};

int main(int argc, char** argv)
{
  loguru::init(argc, argv);
  // setup initial starting point
  scm::initializeSingletons();
  scm::Environment topLevelEnv{};
  scm::setupEnvironment(topLevelEnv);
  std::cout << "scheme interpreter version " << 0.1 << '\n';

  runTests();

  // run function setup for those written in scheme
  std::ifstream functionDefinitionStream;
  functionDefinitionStream.open("/Users/paul/repos/uni/dipl/src/std.scm");
  repl(topLevelEnv, reinterpret_cast<std::istream*>(&functionDefinitionStream), true);

  // define input stream either as cin or from file
  std::istream* streamPtr;
  std::ifstream inputStream;
  std::streambuf* orig_cin = 0;
  bool isFile;
  switch (argc) {
    case 1: {
      // just use the standard input!
      DLOG_F(INFO, "using user input");
      isFile = false;
      streamPtr = &std::cin;
      break;
    }
    case 2: {
      // redirect streambuffer of std::cin to the input file!
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
  repl(topLevelEnv, streamPtr, isFile);

  return 0;
}
