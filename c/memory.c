#define DEBUG

#include "scheme.h"

scmObject SCM_NIL;
scmObject SCM_TRUE;
scmObject SCM_FALSE;

void
initializeSingletons() {
    SCM_NIL = scm_newSingleton(TAG_NIL);
    SCM_TRUE = scm_newSingleton(TAG_TRUE);
    SCM_FALSE = scm_newSingleton(TAG_FALSE);
}


scmObject
scm_newSingleton(scmTag tag) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = tag;
    return p;
}

scmObject
scm_newEnvironment() {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = TAG_ENVIRONMENT;
    p->u.environmentValue.numBindings = 0;
    p->u.environmentValue.bindings = NULL;
    return p;
}

scmObject
scm_newInteger(int iVal) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = TAG_INTEGER;
    p->u.intValue = iVal;
    return p;
}

scmObject
scm_newString(char *characters) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = TAG_STRING;
    p->u.stringValue = characters;
    return p;
}

static int nExistingSymbols = 0;
static scmObject* existingSymbols = NULL;

static void
addToExistingSymbols(scmObject aSymbol) {
    existingSymbols = realloc(existingSymbols,
			       sizeof(scmObject) * (nExistingSymbols + 1));
    existingSymbols[nExistingSymbols] = aSymbol;
    nExistingSymbols++;
}

static scmObject
getExistingSymbolOrNULL(char* characters) {
    for (int i=0; i<nExistingSymbols; i++) {
	scmObject existing = existingSymbols[i];
	if (streq(existing->u.symbolValue, characters)) {
	    return existing;
	}
    }
    return NULL;
}

scmObject
scm_reallyANewSymbol(char *characters) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = TAG_SYMBOL;
    p->u.symbolValue = characters;
    addToExistingSymbols(p);
    return p;
}

scmObject
scm_newSymbol(char *characters) {
    scmObject alreadyExistingSymbol = getExistingSymbolOrNULL(characters);
    scmObject retSym;

    retSym = (alreadyExistingSymbol == NULL)
		? scm_reallyANewSymbol(characters)
		: alreadyExistingSymbol;
    DEBUG_CODE({
	printf("symbol allocated for \"%s\": %p\n", characters, retSym);
    })
    return retSym;
}

scmObject
scm_newCons(scmObject newCar, scmObject newCdr) {
    scmObject p = (scmObject) malloc( sizeof(struct scmObjectStruct) );

    PDEBUG_CODE({
	printf("pointer allocated: %p\n", p);
    })
    p->tag = TAG_CONS;
    p->u.consValue.car = newCar;
    p->u.consValue.cdr = newCdr;
    return p;
}
