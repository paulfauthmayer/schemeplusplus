#pragma once
#include <iostream>
#include <vector>
#include "scheme.hpp"

scm::Object* interpretInput(std::vector<std::string>::iterator& current);
scm::Object* readInput(std::istream* streamPtr);
