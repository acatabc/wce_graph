SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXDIR      = /opt/solvers/cplex1290/cplex
CONCERTDIR    = /opt/solvers/cplex1290/concert

# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC = g++ -O0
CC  = gcc -O0
JAVAC = javac 

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT = -m64 -O -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG -DIL_STD
COPT  = -m64 -fPIC -fno-strict-aliasing
JOPT  = -classpath $(CPLEXDIR)/lib/cplex.jar -O

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXJARDIR   = $(CPLEXDIR)/lib/cplex.jar
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

# For dynamic linking
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(SYSTEM)
CPLEXLIB      = cplex$(dynamic:yes=1290)
run           = $(dynamic:yes=LD_LIBRARY_PATH=$(CPLEXBINDIR))

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) $(dynamic:yes=-L$(CPLEXBINDIR))
CLNDIRS   = -L$(CPLEXLIBDIR) $(dynamic:yes=-L$(CPLEXBINDIR))
CCLNFLAGS = -lconcert -lilocplex -l$(CPLEXLIB) -lm -lpthread -ldl
CLNFLAGS  = -l$(CPLEXLIB) -lm -lpthread -ldl
JAVA      = java  -d64 -Djava.library.path=$(CPLEXDIR)/bin/x86-64_linux -classpath $(CPLEXJARDIR):


CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

EXDIR         = .. 
EXINC         = $(EXDIR)/include
EXLIB        = ../lib
EXSRCC        = $(EXDIR)/src/c
EXSRCCX       = $(EXDIR)/src/c_x
EXSRCCPP      = $(EXDIR)
EXSRCJAVA     = $(EXDIR)/src/java

CFLAGS  = $(COPT)  -I$(CPLEXINCDIR)
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 
JCFLAGS = $(JOPT)

cplex_solver: cplex_solver.o Solver.o WCE_Graph.o utils.o dataReduction.o p3_search.o
	$(CCC) $(CCFLAGS) $(CCLNDIRS) -o cplex_solver WCE_Graph.o utils.o p3_search.o dataReduction.o Solver.o cplex_solver.o  $(CCLNFLAGS)
cplex_solver.o: cplex_solver.cpp
	$(CCC) -c $(CCFLAGS) cplex_solver.cpp -o cplex_solver.o

WCE_Graph.o: $(EXLIB)/WCE_Graph.cpp
	$(CCC) -c $(EXLIB)/WCE_Graph.cpp -o WCE_Graph.o

utils.o: $(EXLIB)/utils.cpp
	$(CCC) -c $(EXLIB)/utils.cpp -o utils.o

dataReduction.o: $(EXLIB)/dataReduction.cpp
	$(CCC) -c $(EXLIB)/dataReduction.cpp -o dataReduction.o

p3_search.o: $(EXLIB)/p3_search.cpp
	$(CCC) -c $(EXLIB)/p3_search.cpp -o p3_search.o

Solver.o: $(EXLIB)/Solver.cpp WCE_Graph.o dataReduction.o p3_search.o utils.o 
	$(CCC) -c  WCE_Graph.o dataReduction.o p3_search.o utils.o $(EXLIB)/Solver.cpp -o Solver.o 


