#ifndef SCHEME_HPP
#define SCHEME_HPP
#include <iostream>

enum scmObjectType {
  SCM_INTEGER = 1,
  SCM_STRING,
  SCM_CONS,
  SCM_NIL,
};

struct scmObjectStruct {
  enum scmObjectType tag;
  union {
    int intValue;
    char* stringValue;
  } u;
};

typedef struct scmObjectStruct* scmObject;
#endif