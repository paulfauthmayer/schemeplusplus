#include "scheme.hpp"

void scm_print(scmObject object)
{
  std::cout << "print tag: " << object->tag << '\n';
  switch (object->tag) {
    case TAG_INTEGER:
      std::cout << object->u.intValue;
      break;
    case TAG_STRING:
      std::cout << '"' << object->u.stringValue << '"';
      break;
    case TAG_SYMBOL:
      std::cout << object->u.stringValue;
      break;
    case TAG_NIL:
      std::cout << "()";
      break;
    case TAG_TRUE:
      std::cout << "#t";
      break;
    case TAG_FALSE:
      std::cout << "#f";
      break;
    default:
      std::cout << "not yet implemented: " << object->tag;
      break;
  }
}