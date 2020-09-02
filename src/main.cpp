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

int main(int argc, char** argv)
{
  // intialise loguru
#ifndef NDEBUG
  loguru::g_stderr_verbosity = loguru::Verbosity_INFO;
#else
  loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
#endif
  loguru::init(argc, argv);

  // setup initial starting point
  scm::initializeSingletons();
  scm::Environment topLevelEnv{};
  scm::setupEnvironment(topLevelEnv);

  // run function setup for those written in scheme
  std::ifstream functionDefinitionStream;
  std::string executableLocationString{argv[0]};
  // get correct file path dependent on os
  std::string executableParentDir;
#if defined(__APPLE__) || defined(__unix__)
  executableParentDir =
      executableLocationString.substr(0, executableLocationString.find_last_of("/"));
  functionDefinitionStream.open(executableParentDir + "/std.scm");
#elif defined(_WIN32) || defined(_WIN64)
  executableParentDir =
      executableLocationString.substr(0, executableLocationString.find_last_of("\\"));
  functionDefinitionStream.open(executableParentDir + "\\std.scm");
#endif
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
      DLOG_IF_F(INFO, scm::LOG_PARSER, "using user input");
      isFile = false;
      streamPtr = &std::cin;
      break;
    }

    // stream from a .scm file
    case 2: {
      DLOG_IF_F(INFO, scm::LOG_PARSER, "parsing input file %s", argv[1]);
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
