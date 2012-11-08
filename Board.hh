#ifndef Board_hh
#define Board_hh

#include "Utils.hh"
#include "PosDir.hh"
#include "Action.hh"

using namespace std;

/**
 * Enum type for all possible contents of a cell.
 */

enum CType {
  Empty, Rock, Capsule, Ball, Kinton, Bean
};

/**
 * Printing function for CType
 */

inline ostream& operator << (ostream& ost, CType t) {
  switch (t) {
  case Empty:    ost << "Empty";   return ost;
  case Rock:     ost << "Rock";    return ost;
  case Capsule:  ost << "Capsule"; return ost;
  case Ball:     ost << "Ball";    return ost;
  case Kinton:   ost << "Kinton";  return ost;
  case Bean:     ost << "Bean";    return ost;
  default:                         return ost;
  }
}


/**
 * Enum type for all possible types of gokus.
 */

enum GType {
  Normal, On_Kinton, With_Ball, On_Kinton_With_Ball
};

/**
 * Printing function for GType
 */

inline ostream& operator << (ostream& ost, GType t) {
  switch (t) {
  case Normal:              ost << "Normal";              return ost;
  case On_Kinton:           ost << "On_Kinton";           return ost;
  case With_Ball:           ost << "With_Ball";           return ost;
  case On_Kinton_With_Ball: ost << "On_Kinton_With_Ball"; return ost;  
  default:                                                return ost;
  }
}


/**
 * Returns if a goku is on a kinton cloud.
 */

inline bool has_kinton(GType type) {
  return type == On_Kinton  or  type == On_Kinton_With_Ball;
}


/**
 * Returns the type of a goku that jumps on a kinton cloud.
 */

inline GType jump_on_kinton(GType type) {
  if      (type == Normal)    return On_Kinton;
  else if (type == With_Ball) return On_Kinton_With_Ball;
  else return type;
}

/**
 * Returns if a goku has taken a dragon ball.
 */

inline bool has_ball(GType type) {
  return type == With_Ball  or  type == On_Kinton_With_Ball;
}

/**
 * Returns the type of a goku that takes a ball.
 */

inline GType take_ball(GType type) {
  if      (type == Normal)    return With_Ball;
  else if (type == On_Kinton) return On_Kinton_With_Ball;
  else return type;
}

/**
 * The description of a cell stores its position,
 * its type (Empty|Rock|Capsule|Ball|Kinton|Bean),
 * and a player identifier (none when id==-1).
 */

struct Cell {
  Pos pos;
  CType type;
  int id;
};


/**
 * The description of a kinton cloud stores its position,
 * a flag indicating whether it is present on the board or not, 
 * and the time left before it regenerates again if not present.
 */

struct Kinton_Cloud {
  Pos pos;
  bool present;
  int time;
};

/**
 * The description of a magic bean stores its position,
 * a flag indicating whether it is present on the board or not, 
 * and the time left before it regenerates again if not present.
 */

struct Magic_Bean {
  Pos pos;
  bool present;
  int time;
};

/**
 * The description of a goku stores its player id, position, type
 * (Normal|On_Kinton|With_Ball|On_Kinton_With_Ball), number of stored
 * balls so far, strength, time before kinton vanishes (only
 * meaningful for types On_Kinton, On_Kinton_With_Ball), alive state
 * and time before the goku becomes alive again (if dead).
 */

struct Goku {
  int id;
  Pos pos;
  GType type;
  int balls;
  int strength;
  int kinton;
  bool alive;
  int time;
};


/**
 * Forward declarations.
 */
class Action;


/**
 * The board.
 */

class Board {

  // Allow access to the private part of Board.
  friend class Game;
  friend class SecGame;

  // Game settings
  int nb_players_;
  int nb_rounds_;
  int nb_capsules_;
  int nb_balls_;
  int nb_beans_;
  int nb_kintons_;
  int goku_regen_time_;
  int bean_regen_time_;
  int kinton_regen_time_;
  int kinton_life_time_;
  int max_strength_;
  int res_strength_;
  int moving_penalty_;
  int kamehame_penalty_;
  int combat_penalty_;

  int rows_;
  int cols_;
  vector<string> names_;

  // Game state
  int round_;
  vector< vector<Cell> > cells_;
  vector<Goku>           gokus_;
  vector<Kinton_Cloud>   kintons_;
  vector<Magic_Bean>     beans_;
  vector<double>         status_;     // cpu status. <-1: dead, 0..1: %of cpu time limit

  int nb_balls_to_be_placed_;

  /**
   * Construct a board by reading first round from a stream.
   */
  Board (istream& is);

  /**
   * Print the board preamble to a stream.
   */
  void print_preamble (ostream& os) const;

  /**
   * Print the board to a stream.
   */
  void print (ostream& os) const;

  /**
   * Computes the next board applying the given actions as to the current board.
   * It also returns the actual actions performed.
   */
  Board next (const vector<Action>& asked, vector<Action>& done) const;

  /**
   * Applies a kamehame throw. Returns true if the action can be done.
   */
  bool apply_throw(int p, AType a, Dir d);

  /**
   * Applies a movement. Returns true if the action can be done.
   */
  bool apply_move(int p, AType a, Dir d);

  /**
   * Places a new ball in a random location. Returns true if successful.
   */
  bool place_new_ball(void);

  /**
   * Checks invariants of the board. For debugging.
   */
  bool ok(void) const;

public:

  /**
   * Empty constructor.
   */
  Board ()
  {   }


  /**
   * Returns the number of rounds in the game.
   */
  inline int nb_rounds () const {
    return nb_rounds_;
  }

  /**
   * Returns the number of players in the game.
   */
  inline int nb_players () const {
    return nb_players_;
  }

  /**
   * Returns the number of capsules in the game.
   */
  inline int nb_capsules () const {
    return nb_capsules_;
  }

  /**
   * Returns the number of balls in the game.
   */
  inline int nb_balls () const {
    return nb_balls_;
  }

  /**
   * Returns the number of beans in the game.
   */
  inline int nb_beans () const {
    return nb_beans_;
  }

  /**
   * Returns the number of kintons in the game.
   */
  inline int nb_kintons () const {
    return nb_kintons_;
  }

  /**
   * Returns the regeneration time (as rounds) of gokus in the game.
   */
  inline int goku_regen_time () const {
    return goku_regen_time_;
  }

  /**
   * Returns the regeneration time (as rounds) of beans in the game.
   */
  inline int bean_regen_time () const {
    return bean_regen_time_;
  }

  /**
   * Returns the regeneration time (as rounds) of kintons in the game.
   */
  inline int kinton_regen_time () const {
    return kinton_regen_time_;
  }

  /**
   * Returns the life time of a kinton (as rounds) in the game.
   */
  inline int kinton_life_time () const {
    return kinton_life_time_;
  }

  /**
   * Returns the maximum possible strength of gokus in the game.
   */
  inline int max_strength () const {
    return max_strength_;
  }

  /**
   * Returns the strength of gokus after being ressurrected in the game.
   */
  inline int res_strength () const {
    return res_strength_;
  }

  /**
   * Returns the penalty in strength after moving on foot.
   */
  inline int moving_penalty () const {
    return moving_penalty_;
  }

  /**
   * Returns the penalty in strength after throwing a kamehame ray.
   */
  inline int kamehame_penalty () const {
    return kamehame_penalty_;
  }

  /**
   * Returns the penalty in strength after a combat.
   */
  inline int combat_penalty () const {
    return combat_penalty_;
  }

  /**
   * Returns the number of rows of the maze in the game.
   */
  inline int rows () const {
    return rows_;
  }

  /**
   * Returns the number of columns of the maze in the game.
   */
  inline int cols () const {
    return cols_;
  }

  /**
   * Returns the current round.
   */
  inline int round () const {
    return round_;
  }

  /**
   * Return whether player is a valid player identifier.
   */
  inline bool player_ok (int player) const {
    return player >= 0 and player < nb_players();
  }

  /**
   * Return whether (i,j) is a position inside the board.
   */
  inline bool pos_ok (int i, int j) const {
    return i >= 0 and i < rows() and j >= 0 and j < cols();
  }

  /**
   * Return whether p is a position inside the board.
   */
  inline bool pos_ok (const Pos& p) const {
    return pos_ok(p.i, p.j);
  }

  /**
   * Return whether (i,j) + d is a position inside the board.
   */
  inline bool pos_ok (int i, int j, Dir d) const {
    return pos_ok(Pos(i, j) + d);
  }

  /**
   * Return whether p+d is a position inside the board.
   */
  inline bool pos_ok (const Pos& p, Dir d) const {
    return pos_ok(p + d);
  }

  /**
   * Returns the position resulting from moving from
   * position p towards direction d.
   */
  inline Pos dest (const Pos& p, Dir d) const {
    Pos q = p + d;
    return q;
  }

  /**
   * Returns the name of a player.
   */
  inline string name (int id) const {
    check(player_ok(id));
    return names_.at(id);
  }

  /**
   * Returns the number of balls stored by a player so far.
   */
  inline int balls (int id) const {
    check(player_ok(id));
    return gokus_.at(id).balls;
  }

  /**
   * Returns the current strength of a player.
   */
  inline int strength (int id) const {
    check(player_ok(id));
    return gokus_.at(id).strength;
  }

  /**
   * Returns the current score of a player.
   */
  inline int score (int id) const {
    check(player_ok(id));
    return balls(id) * (max_strength() + 1) + strength(id);
  }

  /**
   * Returns a reference to the current cpu status of a player.
   */
  inline double& status (int player) {
    return status_[player];
  }

  /**
   * Returns the current cpu status of a player.
   * This is a value from 0 (0%) to 1 (100%) or negative if dead.
   */
  inline double status (int id) const {
    check(player_ok(id));
    return status_.at(id);
  }

  /**
   * Returns the cell at (i, j).
   */
  inline const Cell& cell (int i, int j) const {
    check(pos_ok(i, j));
    return cells_[i][j];
  }

  /**
   * Returns the cell at p.
   */
  inline const Cell& cell (const Pos& p) const {
    return cell(p.i, p.j);
  }

  /**
   * Returns the goku of a player.
   */
  inline const Goku& goku (int id) const {
    check(player_ok(id));
    return gokus_.at(id);
  }

  /**
   * Returns a vector of all beans.
   */
  inline const vector<Magic_Bean>& beans () const {
    return beans_;
  }

  /**
   * Returns a vector of all kintons.
   */
  inline const vector<Kinton_Cloud>& kintons () const {
    return kintons_;
  }

};

#endif
