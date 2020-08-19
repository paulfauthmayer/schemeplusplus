#pragma once
#include <vector>
#include "scheme.hpp"

using InputVector = std::vector<scm::Object*>;

scm::Object* interpretInput(std::vector<std::string>::iterator& current);
InputVector readInput();
