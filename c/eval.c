#include "scheme.h"

scmObject
scm_eval(scmObject expr) {
    switch (expr->tag) {
	case TAG_INTEGER:
	case TAG_STRING:
	case TAG_NIL:
	case TAG_TRUE:
	case TAG_FALSE:
	    return expr;
	    break;

	case TAG_SYMBOL:
	case TAG_CONS:

	default:
	    printf("eval not yet impl: %d\n", expr->tag);
	    break;
    }

    return expr;
}
