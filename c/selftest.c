#include "scheme.h"

static void
__assert(bool ok, char* message, char* filename, int lineNr) {
    if (! ok) {
	printf("assertion failed: %s [%s:%d]\n", message, filename, lineNr);
    }
}

#define assert(b,msg) __assert(b,msg,__FILE__,__LINE__)

void
selftest() {
    scmObject o;

    assert(FALSE, "this should report a failure");

    o = scm_newInteger(0);
    assert(o->tag == SCM_INTEGER, "wrong tag");
    assert(o->u.intValue == 0, "wrong intVal (expected: 0)");

    o = scm_newInteger(-5);
    assert(o->tag == SCM_INTEGER, "wrong tag");
    assert(o->u.intValue == -5, "wrong intVal (expected: -5)");

}
