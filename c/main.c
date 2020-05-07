#include "scheme.h"

#define DEBUG

void
main(int argCount, char** args) {
    selftest();

    printf("hello world, welcome to our scheme 2020\n");

#if defined(DEBUG)
    printf("argCount is %d \n", argCount);
    for (int i=0;i<argCount; i++) {
	printf("arg%d ist %s\n", i, args[i]);
    };
#endif

    for (;;) {
	scmObject expr;
	scmObject val;

	expr = scm_read();
	printf("read returned obj with tag %d\n", expr->tag);
	val = scm_eval(expr);
	scm_print(val);
	printf("\n");
    }
}
