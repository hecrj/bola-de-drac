#ifndef Action_hh
#define Action_hh

#include "Utils.hh"
#include "PosDir.hh"

using namespace std;


/**
 * Enum type for all possible actions.
 */

enum AType {
  Undefined, Moving, Throwing
};

/**
 * Conversion from AType to char.
 */
inline char a2c (AType a) {
  switch (a) {
  case Undefined: return 'u';
  case Moving:    return 'm';
  case Throwing:  return 't';
  }
  _unreachable();
}


/**
 * Conversion from char to AType.
 */
inline AType c2a (char c) {
  switch (c) {
  case 'u':   return Undefined;
  case 'm':   return Moving;
  case 't':   return Throwing;
  }
  return Undefined;
}

/**
 * Class that stores the actions requested by a player in a round.
 */

class Action {

  friend class Game;
  friend class SecGame;
  friend class Board;

  AType type;
  Dir   dir;

  /**
   * Constructor reading one action from a stream.
   */
  Action (istream& is);

  /**
   * Print the action to a stream.
   */
  void print (ostream& os) const;


public:

  /**
   * Creates an empty action.
   */
  Action () : type(Undefined), dir(None)
  {   }

  /**
   * Moves the goku of the player in a given direction.
   */
  void move (Dir d) {
    if (type == Undefined) {
      type = Moving;
      dir = d;
    } else {
      cerr << "warning: moving action already requested for this goku." << endl;
    }   }

  void throw_kamehame (Dir d) {
    if (type == Undefined) {
      type = Throwing;
      dir = d;
    } else {
      cerr << "warning: throwing action already requested for this goku." << endl;
    }   }

};


#endif
