#pragma once
#include "environment.hpp"
namespace scm {
void repl(scm::Environment& env, std::istream* streamPtr, bool isFile = true);
}  // namespace scm
