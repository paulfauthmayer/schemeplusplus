#include "scheme.h"


scmObject
scm_newInteger(int iVal) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    // printf("pointer allocated: %p\n", p);
    p->tag = SCM_INTEGER;
    p->u.intValue = iVal;
    return p;
}
