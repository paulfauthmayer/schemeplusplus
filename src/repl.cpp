#include "repl.hpp"
#include <iostream>
#include <loguru.hpp>
#include "environment.hpp"
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "scheme.hpp"

#if defined(__APPLE__) || defined(__unix__)
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace scm {

/**
 * The heart of this interpreter, the Read Eval Print Loop. Will read an expression,
 * evaluate it and then return the result.
 * @param env the top level environment of the repl
 * @param streamPtr the stream from which to read
 * @param isFile whether we're reading a file or user input
 */
void repl(scm::Environment& env, std::istream* streamPtr, bool isFile)
{
  do {
    try {
      // READ
      // line indicator for user input
      if (!isFile) {
        std::cout << loguru::terminal_red() << "λ " << loguru::terminal_reset();
      }
      scm::Object* expression = scm::readInput(streamPtr, isFile);
      if (expression == SCM_EOF ||
          (scm::hasTag(expression, scm::TAG_CONS) && getCar(expression) == SCM_EOF)) {
        return;
      }

      // handle input cancel by user
      if (expression == SCM_VOID) {
        continue;
      }

      // EVALUATE
      scm::Object* value = scm::trampoline::evaluateExpression(env, expression);

      // PRINT
      // void values should not be printed
      if (value != scm::SCM_VOID) {
        std::cout << "--> " << scm::toString(value) << std::endl;
      }
      if (!isFile) {
        std::cout << '\n';
      }
    }
    catch (scm::schemeException& e) {
      std::cerr << e.what() << '\n';
    }
    catch (std::exception& e) {
      std::cerr << "[CPP::ERROR] " << e.what() << '\n';
    }
  } while (true);  // LOOP!
};

std::string lambdaGraphics =
    "          ////////                                \n\
          /////////         ///          ///      \n\
              //////        ///          ///      \n\
               //////   ///////////  ///////////  \n\
             /////////      ///          ///      \n\
            ///////////     ///          ///      \n\
          //////  //////                          \n\
         //////    //////                         \n\
       //////       //////                        \n\
     //////          //////////                   \n\
    /////.            ///////.                    ";

/**
 * Gets the current width of the console running this program
 * @returns the width of the console as an integer
 */
int getConsoleWidth()
{
  int cols;
#if defined(__APPLE__) || defined(__unix__)
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  cols = w.ws_col;
#elif defined(_WIN32) || defined(_WIN64)
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  int cols;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
  columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
#endif

  return cols;
}

/**
 * Prints a string centered to the console.
 */
void printCentered(std::string content)
{
  int consoleWidth{getConsoleWidth()};
  std::string spaces{};
  for (int i{}; i < (consoleWidth - content.length()) / 2; i++) {
    spaces.push_back(' ');
  }
  std::cout << spaces << content << '\n';
}

/**
 * Prints a welcome message, called after the initial setup has finished.
 */
void printWelcome()
{
  std::stringstream ss(lambdaGraphics);
  std::string token;
  std::cout << "\n\n";
  while (std::getline(ss, token, '\n')) {
    printCentered(token);
  }
  std::cout << "\n\n";
  printCentered("Welcome to Scheme++");
  printCentered("Version 1.0.0");
  printCentered("paulfauthmayer 2020");
  std::cout << "\n\n";
}
}  // namespace scm