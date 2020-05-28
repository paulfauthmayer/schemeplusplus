#include "memory.hpp"
#include "scheme.hpp"

char skipWhitespaces()
{
  char ch;
  // skip all whitespace characters
  do {
    ch = getchar();
  } while ((ch == ' ') or (ch == '\t') or (ch == '\n') or (ch == '\r'));

  return ch;
}

static bool isDigit(char ch)
{
  return ((ch >= '0') and (ch <= '9'));
}

static scmObject scm_readNumber(char firstChar)
{
  char nextChar;
  int iVal{firstChar - '0'};
  nextChar = getchar();
  while (isDigit(nextChar)) {
    iVal = (iVal * 10) + (nextChar - '0');
    nextChar = getchar();
  }

  return scm_newInteger(iVal);
}

static scmObject scm_readString()
{
  // strings are faster in most compilers than char arrays, so should we change
  // this again?
  char nextChar;
  int count{0};
  int bufferSize{64};
  char* stringBuffer = static_cast<char*>(malloc(bufferSize));

  nextChar = getchar();
  while (nextChar != '"') {
    stringBuffer[count++] = nextChar;
    nextChar = getchar();
  }

  // TODO: is this necessary for cpp strings?
  stringBuffer[count++] = '\0';

  std::cout << "read string \"" << stringBuffer << "\"" << std::endl;

  return scm_newString(static_cast<char*>(realloc(stringBuffer, count)));
}

scmObject scm_read()
{
  char ch{skipWhitespaces()};
  std::cout << "got " << ch << " encoded as " << static_cast<int>(ch) << ".\n";

  if (isDigit(ch)) {
    return scm_readNumber(ch);
  }
  else if (ch == '"') {
    std::cout << "reading string!" << std::endl;
    return scm_readString();
  }
  return NULL;
}