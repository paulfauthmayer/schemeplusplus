#include "scheme.hpp"

scmObject scm_eval(scmObject expression)
{
  switch (expression->tag) {
    case TAG_INTEGER:
    case TAG_STRING:
    case TAG_NIL:
    case TAG_TRUE:
    case TAG_FALSE:
      return expression;
      break;

    case TAG_SYMBOL:
    case TAG_CONS:

    default:
      std::cout << "eval not yet implemented for " << expression->tag << '\n';
      break;
  }
  return expression;
}