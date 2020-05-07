#include "scheme.h"

char
skipWhiteSpace() {
    char ch;

    do {
	ch = getchar();
    } while ((ch == ' ')
	     || (ch == '\t')
	     || (ch == '\n')
	     || (ch == '\r'));
    return ch;
}

static int
isDigit(char ch) {
    return ((ch >= '0') && (ch <= '9'));
}

static scmObject
scm_readNumber(char firstChar) {
    char nextChar;
    int iVal = 0;

    iVal = firstChar - '0';
    // printf("iVal %d\n", iVal);
    nextChar = getchar();
    while (isDigit(nextChar)) {
	iVal = (iVal * 10) + (nextChar - '0');
	// printf("iVal %d\n", iVal);
	nextChar = getchar();
    }
    // printf("iVal am ENde %d\n", iVal);
    return scm_newInteger(iVal);
}

static scmObject
scm_readString() {
    char nextChar;
    int count = 0;
    char stringBuffer[64];

    // printf("iVal %d\n", iVal);
    nextChar = getchar();
    while (nextChar != '"') {
	stringBuffer[count++] = nextChar;
	nextChar = getchar();
    }
    stringBuffer[count++] = '\0';
    printf("eingelesener string: %s\n", stringBuffer);
    return NULL; // scm_newString(stringBuffer);
}

scmObject
scm_read() {
    char ch;

    ch = skipWhiteSpace();
    printf("got %c (codiert als %d)\n", ch, ch);
    if (isDigit(ch)) {
	return scm_readNumber(ch);
    }
    if (ch == '"') {
	return scm_readString();
    }
    printf("not yet implemented\n");
    return NULL;
}
