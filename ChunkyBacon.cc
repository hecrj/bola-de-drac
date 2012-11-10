#include "Player.hh"
#include <list>
#include <queue>
#include <limits>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME ChunkyBacon2


typedef vector< vector<int> > Dist;
typedef vector< vector<Pos> > Prev;
typedef list<Pos> Path;
typedef pair<int, Pos> Priority;
typedef priority_queue< Priority, vector<Priority>, greater<Priority> > IndexedQueue;

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
    Path path;

    void push_adjacent(const Pos &u, Dist &d, Prev &p, IndexedQueue &q, Dir dir)
    {
        Pos v = u + dir;

        if(! pos_ok(v))
            return;

        if(cell(v).type == Rock)
            return;

        int cost = 1;
        int players = nb_players();

        for(int i = 0; i < players; ++i)
        {
            if(i != me())
            {
                if(goku(i).pos == u and goku(i).strength >= gme.strength)
                    cost += 20;
            }
        }

        int new_dist = cost + d[u.i][u.j];

        if(new_dist < d[v.i][v.j])
        {
            d[v.i][v.j] = new_dist;
            p[v.i][v.j] = u;
            q.push(make_pair(new_dist, v));
        }
    }

    void generatePath(const Prev &p, Pos u)
    {
        if(u.i == -1)
        {
            path.pop_front();
            return;
        }

        path.push_front(u);
        generatePath(p, p[u.i][u.j]);
    }

    bool recalculatePath(CType c)
    {
        // Clear the current path
        path.clear();

        // Minimum distances to each Cell
        Dist d(rows(), vector<int>(cols(), max_dist));

        // Previous position to make paths
        Prev p(rows(), vector<Pos>(cols()));

        // Pending positions to visit
        IndexedQueue q;

        // ChunkyBacon is here!
        Pos pos = goku(me()).pos;

        d[pos.i][pos.j] = 0;
        p[pos.i][pos.j] = Pos(-1, -1);

        q.push(make_pair(0, pos));

        while(! q.empty())
        {
            Pos u = q.top().second;
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
        Pos d = path.front();
        path.pop_front();

        Dir dir = getDirection(u, d);

        move(dir);
    }

    bool has_turn(const Goku &g)
    {
        return (g.alive and (has_kinton(g.type) or round() % 2 == 0));
    }

    bool is_strength_low(const Goku &g, const Path &p)
    {
        if(obj == Bean and !path.empty())
            return true;

        if(has_kinton(g.type))
            return (g.strength <= (moving_penalty() * ((int)path.size() + 18) + combat_penalty()*4));

        return (g.strength <= (moving_penalty() * ((int)path.size() + 10) + combat_penalty()*2));
    }

    bool is_too_far(const Path &p)
    {
        return (!has_kinton(gme.type) and path.size() > 10);
    }

    bool has_objective_changed()
    {
        if(path.empty())
            return true;

        Pos d = path.back();

        return (cell(d).type != obj);
    }

    void find_new_objective()
    {
        if(is_strength_low(gme, path))
            obj = Bean;

        else if(has_ball(gme.type))
            obj = Capsule;

        else
            obj = Ball;

        recalculatePath(obj);

        if(is_too_far(path))
            recalculatePath(Kinton);
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

        if(! gme.alive)
        {
            path.clear();
            return;
        }

        if(! has_turn(gme))
            return;

        if(gme.strength < moving_penalty())
            path.clear();

        if(has_objective_changed())
            find_new_objective();
        else
            recalculatePath(obj);

        if(path.empty())
        {
            obj = has_kinton(gme.type) ? Bean : Kinton;
            recalculatePath(obj);
        }
        
        followPath();
    }

    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);

