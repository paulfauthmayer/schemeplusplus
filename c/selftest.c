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
    assert(o->tag == TAG_INTEGER, "wrong tag");
    assert(o->u.intValue == 0, "wrong intVal (expected: 0)");

    o = scm_newInteger(-5);
    assert(o->tag == TAG_INTEGER, "wrong tag");
    assert(o->u.intValue == -5, "wrong intVal (expected: -5)");

    o = scm_newSymbol("abc");
    assert(o->tag == TAG_SYMBOL, "wrong tag");
    assert(streq(o->u.symbolValue, "abc"), "wrong symbolVal");

    o = scm_newEnvironment();
    assert(o->tag == TAG_ENVIRONMENT, "wrong tag");
    assert(o->u.environmentValue.numBindings == 0, "wrong nBindings");

    // environment tests
    {
	scmObject key = scm_newSymbol("a");
	scmObject val10 = scm_newInteger(10);
	scmObject val99 = scm_newInteger(99);
	scmObject env;

	env = scm_newEnvironment();
	environment_define(env, key, val10);
	o = environment_get(env, key);
	assert(o == val10, "wrong get value for a");

	environment_define(env, scm_newSymbol("b"), val99);
	o = environment_get(env, scm_newSymbol("b"));
	assert(o == val99, "wrong get value for b");

    }
}
