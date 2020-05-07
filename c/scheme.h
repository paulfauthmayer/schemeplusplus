#include <stdio.h>
#include <stdlib.h>

enum schemeObjectType {
    SCM_INTEGER = 1,
    SCM_STRING,
    SCM_CONS,
    SCM_NIL,
};

// angenommen p ist ptr auf schemeObjekt
// if (p->tag == SCM_INTEGER) ...
//
//     p->u.intValue
//     p->u.stringValue
//

struct scmObjectStruct {
    enum schemeObjectType tag;
    union {
	int intValue;
	char *stringValue;
    } u;
};

typedef struct scmObjectStruct*  scmObject;
typedef int bool;
#define TRUE  1
#define FALSE 0

scmObject scm_read();
scmObject scm_eval(scmObject);
void scm_print(scmObject);
scmObject scm_newInteger(int);
