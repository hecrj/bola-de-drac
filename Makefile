# Add your players here with .o extension.

PLAYERS = ChunkyBacon.o ChunkyBacon2.o ChunkyBacon3.o Demo.o Tonto.o

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

Makefile.deps:
	$(CXX) $(CPPFLAGS) -MM *.cc > Makefile.deps

play: all
	./BolaDeDrac ChunkyBacon3 Tonto Tonto Tonto -i demo.cnf -o game.bdd

play2: all
	./BolaDeDrac ChunkyBacon ChunkyBacon ChunkyBacon ChunkyBacon2 -i demo.cnf -o game.bdd

view:
	./viewer.sh game.bdd

include Makefile.deps
