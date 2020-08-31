#pragma once

#include <iostream>
#include "environment.hpp"
#include "memory.hpp"
#include "scheme.hpp"

namespace scm {

void defineNewSyntax(Environment& env,
                     std::string name,
                     int nArgs,
                     FunctionTag tag,
                     std::string helpText);
void defineNewBuiltinFunction(Environment& env,
                              std::string name,
                              int nArgs,
                              FunctionTag tag,
                              std::string helpText);
void setupEnvironment(Environment& env);

}  // namespace scm
