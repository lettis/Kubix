
include Makefile.def

INCLUDE = -Iinclude

#ENGINE = src/engine
GUI = src/gui
MAIN = src/

MODULES = $(ENGINE) $(GUI)
BIN = kubix


.PHONY: all clean $(MAIN) $(MODULES) doc
all: $(BIN)

$(BIN): $(MAIN) $(MODULES)
	$(CPP) $(CPPFLAGS) -o $(BIN) $(INCLUDE) `find . -maxdepth 10 -name *.o`

$(MAIN) $(MODULES):
	$(MAKE) --no-print-directory --directory=$@ $(TARGET)


doc:
	doxygen doxy.conf
	$(MAKE) --directory=doc/latex

# used to clean directory from binaries
clean:
	@$(MAKE) TARGET=clean cleanhere

cleanhere: $(MAIN) $(MODULES)
	rm $(BIN)

