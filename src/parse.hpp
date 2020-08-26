#pragma once
#include <iostream>
#include <vector>
#include "scheme.hpp"

namespace scm {

Object* interpretInput(std::vector<std::string>::iterator& current);
Object* readInput(std::istream* streamPtr, bool isFile = true);

}  // namespace scm
