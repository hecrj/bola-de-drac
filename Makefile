PLAYERS = ChunkyBacon.o ChunkyBacon2.o ChunkyBacon3.o ChunkyBacon4.o EDI0.o EDI1.o Demo.o Tonto.o
PLAYERS += Meteor0.o Meteor1.o Ultimate2.o Ultimate3.o
PLAYERS += IO.o I0.o I1.o ION.o
PLAYERS += Th0r.o ThorTest.o Thor.o

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
	./BolaDeDrac I1 I0 Thor Th0r -i jaumebosch2.cnf -o game.bdd

view:
	./viewer.sh game.bdd

Makefile.deps:
	$(CXX) $(CPPFLAGS) -MM *.cc > Makefile.deps

include Makefile.deps