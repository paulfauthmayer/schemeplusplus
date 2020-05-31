#include "memory.hpp"
#include "scheme.hpp"

static bool isWhiteSpace(char ch)
{
  return ((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n'));
}

char skipWhitespaces()
{
  char ch;
  // skip all whitespace characters
  do {
    ch = getchar();
  } while (isWhiteSpace(ch));

  return ch;
}

char stream_next(scmInputStream input)
{
  std::cout << "getting next character\n";
  char ch;
  int get_ch;

  ch = input->peekChar;
  std::cout << "peak char: " << ch << '\n';
  if (ch != '\0') {
    input->peekChar = '\0';
    return ch;
  }
  get_ch = fgetc(input->stream);
  if (get_ch < 0) {
    std::cout << "Warning: EOF encountered\n";
    return '\0';
  }

  return get_ch;
}

void stream_unread(scmInputStream input, char ch)
{
  if (input->peekChar != '\0') {
    std::cout << "cannout unread twice\n";
  }
  input->peekChar = ch;
}

static bool isDigit(char ch)
{
  return ((ch >= '0') and (ch <= '9'));
}

static scmObject scm_readNumber(scmInputStream in, char firstChar)
{
  std::cout << "first digit: " << firstChar << '\n';
  char nextChar;
  int iVal{firstChar - '0'};

  nextChar = stream_next(in);
  std::cout << "next digit: " << nextChar << '\n';
  while (isDigit(nextChar)) {
    std::cout << nextChar << '\n';
    iVal = (iVal * 10) + (nextChar - '0');
    nextChar = stream_next(in);
  }

  return scm_newInteger(iVal);
}

static scmObject scm_readString(scmInputStream in)
{
  // strings are faster in most compilers than char arrays, so should we change
  // this again?
  char nextChar;
  int count{0};
  int bufferSize{64};
  char* stringBuffer = static_cast<char*>(malloc(bufferSize));

  nextChar = stream_next(in);
  while (nextChar != '"') {
    if (count >= bufferSize) {
      bufferSize *= 2;
      stringBuffer = static_cast<char*>(realloc(stringBuffer, bufferSize));
    }
    stringBuffer[count++] = nextChar;
    nextChar = getchar();
  }

  // TODO: is this necessary for cpp strings?
  stringBuffer[count++] = '\0';

  std::cout << "read string \"" << stringBuffer << "\"" << std::endl;

  return scm_newString(static_cast<char*>(realloc(stringBuffer, count)));
}

scmObject scm_read(scmInputStream in)
{
  char ch{skipWhitespaces()};
  std::cout << "got " << ch << " encoded as " << static_cast<int>(ch) << ".\n";

  if (isDigit(ch)) {
    std::cout << "read digit\n";
    return scm_readNumber(in, ch);
  }
  else if (ch == '"') {
    std::cout << "reading string!" << std::endl;
    return scm_readString(in);
  }
  return NULL;
}