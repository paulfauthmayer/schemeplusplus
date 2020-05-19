#include "memory.hpp"
#include "scheme.hpp"

static void _assert(bool ok,
                    std::string message,
                    std::string filename,
                    int lineNr)
{
  if (!ok) {
    std::cout << "assertion failed: " << message << " [" << filename << ":"
              << lineNr << "]\n";
  }
};

// TODO: should I replace this with the default cpp assert function?
#define assert(b, msg) _assert(b, msg, __FILE__, __LINE__)

void selftest()
{
  scmObject obj;

  assert(false, "this should report a failure");

  obj = scm_newInteger(0);
  assert(obj->tag == TAG_INTEGER, "wrong tag");
  assert(obj->u.intValue == 0, "wrong intVal (expected: 0)");

  obj = scm_newInteger(-5);
  assert(obj->tag == TAG_INTEGER, "wrong_tag");
  assert(obj->u.intValue == -5, "wrong intVal (exprected: -5)");
}