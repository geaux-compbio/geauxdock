
include ../GNUmakefile.common.mk


EXE := geauxdock
OBJ += geauxdock.o
OBJ += backend_cpu.o
OBJ += util_optimize_soa_cpu.o
OBJ += load.o data.o rmsd.o util.o util_print.o

default: $(EXE)

$(EXE): $(OBJ)
	$(CXX_HOST) $(LIBPATH) $(OBJ) -o $@ $(LINKFLAGS)

geauxdock.o: geauxdock.C
	$(CXX_HOST) $(CXXFLAGS_HOST) -c $< -o $@



NT := 1 # number of temperatures passing to the runtime
include ../GNUmakefile.common.rules

