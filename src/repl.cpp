#include "repl.hpp"
#include <iostream>
#include "environment.hpp"
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "scheme.hpp"

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
        std::cout << "> ";
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
    }
    catch (scm::schemeException& e) {
      std::cerr << e.what() << '\n';
    }
    catch (std::exception& e) {
      std::cerr << "[CPP::ERROR] " << e.what() << '\n';
    }
  } while (true);  // LOOP!
};

}  // namespace scm