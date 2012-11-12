
#include "Player.hh"

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Null





struct PLAYER_NAME : public Player {

  static const int DF[4];
  static const int DC[4];

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


    /**
     * Play method.
     * 
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     */     
    virtual void play () {
    }

    
};

const int PLAYER_NAME::DF[4] = { 0, 0, 1,-1};
const int PLAYER_NAME::DC[4] = { 1,-1, 0, 0};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

