#include "printer.hpp"
#include "memory.hpp"
#include "scheme.hpp"

static void scm_printList(scmObject list)
{
  scm_print(list->u.consValue.car);
  if (list->u.consValue.cdr == SCM_NIL) {
    std::cout << ')';
    return;
  }
  else {
    std::cout << ' ';
    scm_printList(list->u.consValue.cdr);
  }
}

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
      std::cout << object->u.symbolValue;
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
    case TAG_CONS:
      std::cout << '(';
      scm_printList(object);
      break;
    default:
      std::cout << "not yet implemented: " << object->tag;
      break;
  }
}