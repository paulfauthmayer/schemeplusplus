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
  char nextChar;
  // TODO: test whether this is slower han gittinger's solution
  std::string stringBuffer;

  nextChar = getchar();
  while (nextChar != '"') {
    stringBuffer.push_back(nextChar);
    nextChar = getchar();
  }

  stringBuffer.push_back('\0');
  // TODO: remove endl, this is only used because this wouldn't print anything
  // otherwise
  std::cout << "read string \"" << stringBuffer << "\"" << std::endl;

  // TODO: this should return a scmObject in the future --> implement in eval
  return NULL;
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