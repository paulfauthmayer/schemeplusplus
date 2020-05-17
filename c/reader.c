#include "scheme.h"

char
stream_next(scmInputStream input) {
    char ch;
    int get_ch;

    ch = input->peekChar;
    if (ch != '\0') {
	input->peekChar = '\0';
	return ch;
    }
    get_ch = fgetc(input->stream);
    if (get_ch < 0) {
	printf("warning: EOF\n");
	return '\0';
    }
    return get_ch;
}

char
stream_unread(scmInputStream input, char ch) {
    if (input->peekChar != '\0') {
	printf("oops - 2 mal unread geht nicht\n");
    }
    input->peekChar = ch;
}

static bool
isWhiteSpace(char ch) {
    return ((ch == ' ')
	     || (ch == '\t')
	     || (ch == '\n')
	     || (ch == '\r'));
}

char
skipWhiteSpace(scmInputStream in) {
    char ch;

    do {
	ch = stream_next(in);
    } while (isWhiteSpace(ch));
    return ch;
}

static int
isDigit(char ch) {
    return ((ch >= '0') && (ch <= '9'));
}

static scmObject
scm_readNumber(scmInputStream in, char firstChar) {
    char nextChar;
    int iVal = 0;

    iVal = firstChar - '0';
    // printf("iVal %d\n", iVal);
    nextChar = stream_next(in);
    while (isDigit(nextChar)) {
	iVal = (iVal * 10) + (nextChar - '0');
	// printf("iVal %d\n", iVal);
	nextChar = stream_next(in);
    }
    // printf("iVal am ENde %d\n", iVal);
    stream_unread(in, nextChar);
    return scm_newInteger(iVal);
}

static scmObject
scm_readString(scmInputStream in) {
    char nextChar;
    int count = 0;
    int bufferSize = 64;
    char *stringBuffer = malloc(bufferSize);

    // printf("iVal %d\n", iVal);
    nextChar = stream_next(in);
    while (nextChar != '"') {
	if (count >= bufferSize) {
	    printf("buffer resize\n");
	    stringBuffer = realloc(stringBuffer, bufferSize*2);
	    bufferSize *= 2;
	}
	stringBuffer[count++] = nextChar;
	nextChar = stream_next(in);
    }
    stringBuffer[count++] = '\0';
    printf("eingelesener string: \"%s\"\n", stringBuffer);

    return scm_newString(realloc(stringBuffer, count));
}

static scmObject
scm_readSymbol(scmInputStream in, char chAlreadyRead) {
    char nextChar;
    int count = 0;
    int bufferSize = 64;
    char *stringBuffer = malloc(bufferSize);

    stringBuffer[count++] = chAlreadyRead;

    // printf("iVal %d\n", iVal);
    nextChar = stream_next(in);
    while (! isWhiteSpace(nextChar)
	   && (nextChar != '(')
	   && (nextChar != ')')
	   && (nextChar != '"')
    ) {
	if (count >= bufferSize) {
	    printf("buffer resize\n");
	    stringBuffer = realloc(stringBuffer, bufferSize*2);
	    bufferSize *= 2;
	}
	stringBuffer[count++] = nextChar;
	nextChar = stream_next(in);
    }
    stringBuffer[count++] = '\0';

    if (streq(stringBuffer, "#t")) {
	return SCM_TRUE;
    }
    if (streq(stringBuffer, "#f")) {
	return SCM_FALSE;
    }
    printf("eingelesenes symbol: %s\n", stringBuffer);

    return scm_newSymbol(realloc(stringBuffer, count));
}

static scmObject
scm_readList(scmInputStream in) {
    char ch;
    scmObject element, restListe;

    DEBUG_CODE({ printf("enter readList\n", ch); })

    ch = skipWhiteSpace(in);
    if (ch == ')') {
	return SCM_NIL;
    }

    stream_unread(in, ch);
    element = scm_read(in);
    restListe = scm_readList(in);
    return scm_newCons(element, restListe);
}

scmObject
scm_read(scmInputStream in) {
    char ch;

    ch = skipWhiteSpace(in);
    DEBUG_CODE({ printf("got %c (codiert als %d)\n", ch, ch); })
    if (isDigit(ch)) {
	return scm_readNumber(in, ch);
    }
    if (ch == '"') {
	return scm_readString(in);
    }
    if (ch == '(') {
	return scm_readList(in);
    }
    // anything else
    return scm_readSymbol(in, ch);
}
