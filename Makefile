PLAYERS = ChunkyBacon.o ChunkyBacon2.o ChunkyBacon3.o ChunkyBacon4.o EDI0.o EDI1.o Demo.o Tonto.o
PLAYERS += Meteor0.o Meteor1.o Ultimate2.o Ultimate3.o
PLAYERS += IO.o

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
	./BolaDeDrac IO Meteor1 Meteor1 IO -i cnord.cnf -o game.bdd

view:
	./viewer.sh game.bdd

Makefile.deps:
	$(CXX) $(CPPFLAGS) -MM *.cc > Makefile.deps

include Makefile.deps