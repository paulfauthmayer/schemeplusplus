#define DEBUG

#include "scheme.h"

void
environment_define(scmObject env, scmObject key, scmObject newVal) {
    int nBindings = env->u.environmentValue.numBindings;
    scmObject* newBindings;

    DEBUG_CODE({ printf("define n: %d\n", nBindings); })
    for (int i=0; i<nBindings; i++) {
	scmObject binding = env->u.environmentValue.bindings[i];
	scmObject oldKey = binding->u.consValue.car;
	DEBUG_CODE({
	    printf("define check old Key: ");
	    scm_print(oldKey);
	    printf("\n");
	})
	if (oldKey == key) {
	    DEBUG_CODE({
		printf("define change oldKey value to: ");
		scm_print(newVal);
		printf("\n");
	    })
	    binding->u.consValue.cdr = newVal;
	    return;
	}
    }

    DEBUG_CODE({
	printf("define new binding for ");
	scm_print(key);
	printf(" with value: ");
	scm_print(newVal);
	printf(" at index: %d\n", nBindings);
    })
    newBindings = realloc(env->u.environmentValue.bindings,
			  sizeof(scmObject) * (nBindings+1));
    newBindings[nBindings] = scm_newCons(key, newVal);
    env->u.environmentValue.bindings = newBindings;
    env->u.environmentValue.numBindings++;
}

scmObject
environment_get(scmObject env, scmObject key)
{
    int nBindings = env->u.environmentValue.numBindings;

    DEBUG_CODE({ printf("get n: %d\n", nBindings); })
    for (int i=0; i<nBindings; i++) {
	scmObject binding = env->u.environmentValue.bindings[i];
	scmObject oldKey = binding->u.consValue.car;
	DEBUG_CODE({
	    printf("get check old Key: ");
	    scm_print(oldKey);
	    printf("\n");
	})
	if (oldKey == key) {
	    return binding->u.consValue.cdr;
	}
    }
    DEBUG_CODE({
	printf("nothing found\n");
    })
    // not found
    return NULL;
}
