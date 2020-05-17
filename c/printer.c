#include "scheme.h"

static void
scm_printList(scmObject aList) {
    scm_print(aList->u.consValue.car);
    if (aList->u.consValue.cdr == SCM_NIL) {
	printf(")");
	return;
    }
    printf(" ");
    scm_printList(aList->u.consValue.cdr);
}

void
scm_print(scmObject anyScmObject) {
    // printf("zu printen: objekt mit tag %d", anyScmObject->tag);
    switch (anyScmObject->tag) {
	case TAG_INTEGER:
	    printf("%d", anyScmObject->u.intValue);
	    break;
	case TAG_STRING:
	    printf("\"%s\"", anyScmObject->u.stringValue);
	    break;
	case TAG_SYMBOL:
	    printf("%s", anyScmObject->u.symbolValue);
	    break;
	case TAG_NIL:
	    printf("()");
	    break;
	case TAG_TRUE:
	    printf("#t");
	    break;
	case TAG_FALSE:
	    printf("#f");
	    break;
	case TAG_CONS:
	    printf("(");
	    scm_printList(anyScmObject);
	    break;

	default:
	    printf("print not yet impl: %d\n", anyScmObject->tag);
	    break;
    }
}
