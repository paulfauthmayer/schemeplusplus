#pragma once
#include <iostream>
#include <stack>
#include <vector>
// #include "environment.hpp"

namespace scm {
class Environment;

/**
 * The base class of every object that's supposed to be visible to the garbage collector.
 */
class Collectable {
 public:
  // a unique identifier of the object
  long id;
  // essential objects are never collected
  bool essential;
  // determines whether the object should be spared during the next sweeping cycle
  bool marked;

  Collectable();
  virtual ~Collectable();
};

void markAndSweep(Environment& env);
void mark(Environment& env);

}  // namespace scm
