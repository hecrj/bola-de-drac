#include "Player.hh"

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Demo





struct PLAYER_NAME : public Player {


  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }


  /**
   * Attributes for your player can be defined here.
   */
  Dir dir;

  /**
   * Play method.
   *
   * This method will be invoked once per each round.
   * You have to read the board here to place your actions
   * for this round.
   *
   */
  virtual void play () {
    // initialization
    if (round() == 0) dir = rand_dir(None);

    // move goku
    Pos p = goku(me()).pos;
    if (randomize() % 2 == 0) throw_kamehame(dir);
    else {
      if (randomize() % 100 == 0 or not can_move(p, dir)) dir = rand_dir(dir);
      move(dir);
    }
}

  bool can_move (Pos p, Dir d) {
    CType t = cell(dest(p, d)).type;
    if (t == Rock) return false;
    int id = cell(dest(p, d)).id;
    if (id != -1 and goku(me()).strength < goku(id).strength) return false;
    return true;
  }

Dir rand_dir (Dir notd) {
  Dir a[] = {Left, Right, Bottom, Top};
  while (true) {
    Dir d = a[randomize() % 4];
    if (d != notd) return d;
  }
}

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
