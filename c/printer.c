#include "scheme.h"

void
scm_print(scmObject anyScmObject) {
    // printf("zu printen: objekt mit tag %d", anyScmObject->tag);
    switch (anyScmObject->tag) {
	case SCM_INTEGER:
	    printf("%d", anyScmObject->u.intValue);
	    break;
	case SCM_STRING:
	    printf("%s", anyScmObject->u.stringValue);
	    break;
	    break;
	default:
	    printf("not yet impl: %d\n", anyScmObject->tag);
	    break;
    }
}
