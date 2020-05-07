# initial makefile, automatically generated.
#
# windows object files have a ".obj" suffix / unix uses ".o";
# change the following to "O=obj" for ms-windows
O=o
# windows executables have a ".exe" suffix / unix does not
# change the following to "EXE=.exe" for ms-windows
EXE=.exe
CC="C:\Program Files\MINGW64\bin\gcc.exe"

#
# change OBJS, HEADERS and TARGET as appropriate
# eg.
#   OBJS=file1.$(O) file2.$(O) ...
#   HEADERS=inclFile1.h inclFile2.h ...
#   TARGET=myProg

TARGET=scheme
OBJS=main.$(O) reader.$(O) printer.$(O) eval.$(O) memory.$(O) \
    selftest.$(O)
HEADERS=scheme.h

#
# usually no need to change below
#

OPT="-g" $(CONFIG_OPT)
OPT_SPEED="-Ofast -fomit-frame-pointer -funroll-loops $(CONFIG_OPT)"
# CFLAGS=$(OPT) -pedantic -Wall -Wextra -Wno-unused-parameter
CFLAGS=$(OPT) -pedantic -Wall -Wextra

all: $(TARGET)$(EXE)

debug:
	$(MAKE) CFLAGS=-g -DDEBUG

opt:
	$(MAKE) OPT=$(OPT_SPEED)

$(TARGET)$(EXE): $(OBJS)
	$(CC) -o $(TARGET)$(EXE) $(OBJS)

$(OBJS): $(HEADERS) Makefile

clean:
	-del $(OBJS)

clobber: clean
	-del $(TARGET) cscope.out

.c.o:
	$(CC) -c $*.c
