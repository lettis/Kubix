
# use the gcc as compiler
CC	= gcc

# show all warnings

# debugging flags
CFLAGS	=-Wall -Wextra -pedantic    -ggdb

# optimization
#CFLAGS	=-Wall -Wextra -pedantic -O2


# define all object files
objects = engine.o controller.o rating.o

# build the engine
engine: $(objects)
	$(CC) -o engine $(objects)


# build the single object files (by implicit rule)
engine.o:	engine.h rating.h controller.h
rating.o:	engine.h rating.h
terminal.o:	engine.h controller.h


# used to clean directory from binaries
.PHONY: clean
clean:
	-rm engine $(objects)


