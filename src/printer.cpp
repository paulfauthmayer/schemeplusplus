#include "scheme.hpp"

void scm_print(scmObject object)
{
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
    default:
      std::cout << "not yet implemented: " << object->tag;
      break;
  }
}