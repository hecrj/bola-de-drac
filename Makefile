# Add your players here with .o extension.

PLAYERS = ChunkyBacon.o ChunkyBacon2.o ChunkyBacon3.o ChunkyFSM.o Demo.o Tonto.o

# Do not modify past this point.

EXE = BolaDeDrac

LDLIBS = -lm

#Debug
#CXXFLAGS = -O0 -Wall
#LDFLAGS = -g -rdynamic

#Optimize
CXXFLAGS = -O2 -Wall
LDFLAGS =


all: $(EXE)

clean:
	rm -f $(EXE) *.o *.exe Makefile.deps

BolaDeDrac: BackTrace.o Utils.o PosDir.o Board.o Action.o Player.o Registry.o Game.o BolaDeDrac.o $(PLAYERS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

play: all
	./BolaDeDrac ChunkyBacon3 ChunkyFSM ChunkyBacon2 ChunkyBacon -i demo.cnf -o game.bdd

view:
	./viewer.sh game.bdd

Makefile.deps:
	$(CXX) $(CPPFLAGS) -MM *.cc > Makefile.deps

include Makefile.deps
