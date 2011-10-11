
include ../../Makefile.def

INCLUDE = -I../../include
SRC = engine.cpp

.PHONY: clean

OBJ = $(SRC:%.cpp=%.o)

all: $(OBJ)

%.o: %.cpp
	$(CPP) $(CPPFLAGS) $(INCLUDE) -c $<

clean:
	rm -f $(OBJ)

