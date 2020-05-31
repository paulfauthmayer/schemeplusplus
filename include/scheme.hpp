#ifndef SCHEME_HPP
#define SCHEME_HPP

#include <iostream>

struct scmInputStreamStruct {
  FILE* stream;
  char peekChar;
};

typedef struct scmInputStreamStruct* scmInputStream;

enum scmObjectType {
  TAG_INTEGER = 1,
  TAG_STRING,
  TAG_CONS,
  TAG_NIL,
  TAG_SYMBOL,
  TAG_TRUE,
  TAG_FALSE,
  TAG_ENVIRONMENT,
};

typedef enum scmObjectType scmTag;

struct scmObjectStruct {
  scmTag tag;
  union {
    int intValue;
    char* stringValue;
  } u;
};

typedef struct scmObjectStruct* scmObject;

#endif