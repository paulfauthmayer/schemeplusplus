#include <iostream>
#include "eval.hpp"
#include "printer.hpp"
#include "reader.hpp"
#include "scheme.hpp"
#include "selftest.hpp"

#define DEBUG

scmInputStream SCM_STDIN;

static void initializeStreams()
{
  SCM_STDIN = static_cast<scmInputStream>(malloc(sizeof(struct scmInputStreamStruct)));

  SCM_STDIN->stream = stdin;
  SCM_STDIN->peekChar = '\0';
}

int main(int argc, char** argv)
{
  std::cout << "scheme:cpp version X.Y.Z\n";

  initializeStreams();

#ifdef DEBUG
  std::cout << argc << " arguments were passed\n";
  for (int i{0}; i < argc; i++) {
    std::cout << "arg" << i << ": " << argv[i] << '\n';
  }
  selftest();
#endif

  while (true) {
    // read and interpret the next expression
    scmObject expr{scm_read(SCM_STDIN)};
    std::cout << "read returned object with tag " << expr->tag << '\n';
    scmObject val{scm_eval(expr)};
    scm_print(val);
    std::cout << '\n';
  }

  return 0;
}