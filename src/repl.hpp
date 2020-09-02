#pragma once
#include "environment.hpp"
namespace scm {
void printWelcome();
void repl(scm::Environment& env, std::istream* streamPtr, bool isFile = true);
}  // namespace scm
