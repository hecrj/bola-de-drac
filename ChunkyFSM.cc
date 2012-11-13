#include "Player.hh"
#include <list>
#include <queue>
#include <limits>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME ChunkyFSM

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
    typedef vector< vector<int> > Dist;
    typedef vector< vector<Pos> > PathMap;
    typedef list<Pos> Path;
    typedef pair<int, Pos> Priority;
    typedef priority_queue< Priority, vector<Priority>, greater<Priority> > IndexedQueue;

    class State
    {
        public:
            virtual ~State(){}
            virtual void enter(PLAYER_NAME* player){};
            virtual void execute(PLAYER_NAME* player){};
            virtual void leave(PLAYER_NAME* player){};
    };

    enum SType
    {
        GetBall, GetBean, GetKinton, GoToCapsule
    };

    class PathFinder
    {
        PathMap map;
        Dist distances;

        public:
            void find_near_items(PLAYER_NAME* player, vector<CType> items)
            {
                items = vector<CType>(4, 0);

                
            }

        private:
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
                        Goku g = goku(i);

                        if(g.pos == u and
                            (double(1 + gme.strength) / double(2 + g.strength + gme.strength)) < 0.8)
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

    };

    Goku gme;
    Path path;
    State* global_state;
    SType current_state;
    SType previous_state;
    vector<State*> states;

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

    bool recalculate_path(CType c)
    {
        // Clear the current path
        path.clear();

        // Minimum distances to each Cell
        Dist d(rows(), vector<int>(cols(), numeric_limits<int>::max()));

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

    bool has_turn()
    {
        return (gme.alive and (has_kinton(gme.type) or round() % 2 == 0));
    }

    bool has_strength_low()
    {
        if(has_kinton(gme.type))
            return (gme.strength <= (moving_penalty() * ((int)path.size() + 6)));

        return (gme.strength <= (moving_penalty() * ((int)path.size() + 4)));
    }

    bool is_too_far()
    {
        return (!has_kinton(gme.type) and path.size() > 20);
    }

    void follow_path()
    {
        if(path.empty())
            return;

        Pos u = goku(me()).pos;
        Pos d = path.front();
        
        Dir dir = getDirection(u, d);

        path.pop_front();
        move(dir);
    }

    void to_previous_state()
    {
        states[current_state]->leave(this);

        SType temp = current_state;
        current_state = previous_state;
        previous_state = temp;

        states[current_state]->enter(this);
    }

    bool is_in_state(SType s)
    {
        return current_state == s;
    }

    void change_state(SType s)
    {
        states[current_state]->leave(this);
        previous_state = current_state;

        current_state = s;
        states[current_state]->enter(this);
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

        if(round() == 0)
        {
            path.clear();
            states = vector<State*>(4);
            states[GetBall] = new GetBallState();
            states[GetBean] = new GetBeanState();
            states[GetKinton] = new GetKintonState();
            states[GoToCapsule] = new GoToCapsuleState();

            global_state = new GlobalPlayerState();
            current_state = GetBall;
            states[current_state]->enter(this);
        }

        global_state->execute(this);

        if(has_turn())
            states[current_state]->execute(this);
    }

    class GlobalPlayerState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            if(!player->is_in_state(GetBean) and player->has_strength_low())
                player->change_state(GetBean);

            else if(!player->is_in_state(GetKinton) and player->is_too_far())
                player->change_state(GetKinton);
        }
    };

    class GetBallState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            player->recalculate_path(Ball);
            player->follow_path();

            if(has_ball(player->gme.type))
                player->change_state(GoToCapsule);

            else if(player->path.empty())
            {
                if(has_kinton(player->gme.type))
                    player->change_state(GetBean);
                else
                    player->change_state(GetKinton);
            }
        }
    };

    class GetBeanState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            player->recalculate_path(Bean);
            player->follow_path();

            if(player->path.empty())
                player->to_previous_state();
        }
    };

    class GetKintonState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            player->recalculate_path(Kinton);
            player->follow_path();

            if(player->path.empty())
            {
                if(has_ball(player->gme.type))
                    player->change_state(GetBall);
                else
                    player->change_state(GoToCapsule);
            }
        }
    };

    class GoToCapsuleState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            player->recalculate_path(Capsule);
            player->follow_path();

            if(! has_ball(player->gme.type))
                player->change_state(GetBall);
        }
    };
    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
