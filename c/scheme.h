#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct scmInputStreamStruct {
    FILE* stream;
    char peekChar;
};

typedef struct scmInputStreamStruct* scmInputStream;


enum schemeObjectType {
    TAG_INTEGER = 1,
    TAG_STRING,
    TAG_SYMBOL,
    TAG_CONS,
    TAG_NIL,
    TAG_TRUE,
    TAG_FALSE,
    TAG_ENVIRONMENT,
};

// angenommen p ist ptr auf schemeObjekt
// if (p->tag == SCM_INTEGER) ...
//
//     p->u.intValue
//     p->u.stringValue
//

typedef enum schemeObjectType scmTag;

typedef struct scmObjectStruct* scmObject;

struct scmObjectStruct {
    scmTag tag;
    union {
	int intValue;
	char *stringValue;
	char *symbolValue;
	struct {
	    scmObject car;
	    scmObject cdr;
	} consValue;
	struct {
	    int numBindings;
	    scmObject* bindings;
	} environmentValue;
    } u;
};

typedef int bool;
#define TRUE  1
#define FALSE 0

#define streq(a,b) (strcmp(a,b) == 0)

#ifdef POINTER_DEBUG
# define PDEBUG_CODE(code) code
#else
# define PDEBUG_CODE(code) /* as nothing */
#endif

#ifdef DEBUG
# define DEBUG_CODE(code) code
#else
# define DEBUG_CODE(code) /* as nothing */
#endif


extern scmObject SCM_NIL;
extern scmObject SCM_TRUE;
extern scmObject SCM_FALSE;

extern scmInputStream SCM_STDIN;

extern scmObject scm_read();
extern scmObject scm_eval(scmObject);
extern void scm_print(scmObject);
extern scmObject scm_newInteger(int);
extern scmObject scm_newString(char*);
extern scmObject scm_newSymbol(char*);
extern scmObject scm_newSingleton(scmTag);
extern scmObject scm_newCons(scmObject, scmObject);
extern scmObject scm_newEnvironment();
extern void environment_define(scmObject env, scmObject key, scmObject val);
extern scmObject environment_get(scmObject env, scmObject key);
extern void initializeSingletons();
extern void selftest();
