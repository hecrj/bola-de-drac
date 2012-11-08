#include "Player.hh"
#include <stack>
#include <limits>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME ChunkyBacon


typedef vector< vector<int> > Dist;
typedef vector< vector<Pos> > Prev;

const int max_dist = numeric_limits<int>::max();

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
    Goku gme;
    CType obj;
    stack<Pos> path;

    void push_adjacent(const Pos &u, Dist &d, Prev &p, queue<Pos> &q, Dir dir)
    {
        Pos v = u + dir;

        if(! pos_ok(v))
            return;

        if(cell(v).type == Rock)
            return;

        for(int i = nb_players()-1; i >= 0; --i)
        {
            Goku foe = goku(i);

            if(foe.pos == v and foe.strength >= gme.strength)
                return;
        }

        int new_dist = 1 + d[u.i][u.j];

        if(new_dist < d[v.i][v.j])
        {
            d[v.i][v.j] = new_dist;
            p[v.i][v.j] = u;
            q.push(v);
        }
    }

    void generatePath(const Prev &p, Pos u)
    {
        if(u.i == -1)
        {
            path.pop();
            return;
        }

        path.push(u);
        generatePath(p, p[u.i][u.j]);
    }

    bool recalculatePath(CType c)
    {
        // Clear the current path
        path = stack<Pos>();

        // Minimum distances to each Cell
        Dist d(rows(), vector<int>(cols(), max_dist));

        // Previous position to make paths
        Prev p(rows(), vector<Pos>(cols()));

        // Pending positions to visit
        queue<Pos> q;

        // ChunkyBacon is here!
        Pos pos = goku(me()).pos;

        d[pos.i][pos.j] = 0;
        p[pos.i][pos.j] = Pos(-1, -1);

        q.push(pos);

        while(! q.empty())
        {
            Pos u = q.front();
            q.pop();

            if(cell(u).type == c)
            {
                generatePath(p, u);
                return true;
            }

            for(int i = Top; i <= Right; ++i)
                push_adjacent(u, d, p, q, static_cast<Dir>(i));
        }

        return false;
    }

    Dir getDirection(const Pos &u, const Pos &d)
    {
        for(int i = Top; i <= Right; ++i)
        {
            Dir dir = static_cast<Dir>(i);

            if(u + dir == d)
                return dir;
        }

        return None;
    }

    void followPath()
    {
        if(path.empty())
            return;

        Pos u = goku(me()).pos;
        Pos d = path.top();
        path.pop();

        cout << u.i << ' ' << u.j << endl;
        cout << d.i << ' ' << d.j << endl;

        Dir dir = getDirection(u, d);

        cout << static_cast<int>(dir) << endl;
        move(dir);
    }

    /**
     * Play method.
     * 
     * This method will be invoked once per each round.
     * You have to read the board here to place your actions
     * for this round.
     *
     */     
    virtual void play()
    {
        gme = goku(me());

        if(! has_kinton(gme.type) and round() % 2 != 0)
            return;

        if(has_ball(gme.type))
            obj = Capsule;
        else
            obj = Ball;

        recalculatePath(obj);

        if(! has_kinton(gme.type) and path.size() > 10)
            recalculatePath(Kinton);
        else if(strength(me()) <= (signed int)(moving_penalty() * (path.size() + 8)))
            recalculatePath(Bean);
        
        if(gme.alive)
            followPath();
    }

    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

