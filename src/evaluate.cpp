#include <iostream>
#include "memory.hpp"
#include "scheme.hpp"

scm::Object* evaluate(scm::Object* obj)
{
  scm::Object* evaluatedObj;

  switch (obj->tag) {
    case scm::TAG_INT:
    case scm::TAG_FLOAT:
    case scm::TAG_STRING:
    case scm::TAG_NIL:
    case scm::TAG_FALSE:
    case scm::TAG_TRUE:
      return obj;
      break;
    case scm::TAG_SYMBOL:
    case scm::TAG_CONS:
    default:
      throw("evaluation not yet implemented for " + scm::toString(obj));
  }

  return SCM_NIL;
}