#include "repl.hpp"
#include <iostream>
#include "environment.hpp"
#include "evaluate_trampoline.hpp"
#include "memory.hpp"
#include "parse.hpp"
#include "scheme.hpp"

namespace scm {

void repl(scm::Environment& env, std::istream* streamPtr, bool isFile)
{
  do {
    try {
      // READ
      if (!isFile) {
        std::cout << "> ";
      }
      scm::Object* expression = scm::readInput(streamPtr, isFile);
      if (expression == SCM_EOF ||
          (scm::hasTag(expression, scm::TAG_CONS) && getCar(expression) == SCM_EOF)) {
        return;
      }

      // EVALUATE
      scm::Object* value = scm::trampoline::evaluateExpression(env, expression);

      // PRINT
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