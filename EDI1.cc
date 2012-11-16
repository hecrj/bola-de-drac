#include "Player.hh"
#include <limits>
#include <queue>
#include <cmath>
#include <list>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME EDI1

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
    typedef vector< vector<int> > CostTable;
    typedef vector< vector<Pos> > NodeTable;
    typedef list<Pos> Path;
    typedef queue<Pos> IndexedQueue;

    class State
    {
        public:
            virtual ~State(){}
            virtual void enter(PLAYER_NAME* player){};
            virtual void transition(PLAYER_NAME* player){};
            virtual void execute(PLAYER_NAME* player){};
            virtual void leave(PLAYER_NAME* player){};
    };

    enum SType
    {
        CollectBalls, RecoverEnergy
    };

    class Dijkstra
    {
        PLAYER_NAME* player;
        PLAYER_NAME* board; // Player is the board at the same time D:
        NodeTable map;
        CostTable costs;
        IndexedQueue pending;

        void push_adjacent(const Pos &u, Dir dir)
        {
            Pos v = u + dir;

            if(not board->pos_ok(v))
                return;

            if(board->cell(v).type == Rock)
                return;

            int new_cost = 1 + costs[u.i][u.j];

            if(new_cost < costs[v.i][v.j])
            {
                costs[v.i][v.j] = new_cost;
                map[v.i][v.j] = u;
                pending.push(v);
            }
        }

        void generate_path(Pos u, Path &path)
        {
            if(u.i == -1)
            {
                path.pop_front();
                return;
            }

            path.push_front(u);
            generate_path(map[u.i][u.j], path);
        }

    public:
        Dijkstra(PLAYER_NAME* pboard)
        {
            this->board = pboard;
            this->player = pboard;
        }

        bool search(CType item, Path &path)
        {
            path.clear();

            // Minimum distances to each Cell
            costs = CostTable(board->rows(), vector<int>(board->cols(), numeric_limits<int>::max()));

            // Previous position to make paths
            map = NodeTable(board->rows(), vector<Pos>(board->cols()));

            // Pending positions to visit
            pending = IndexedQueue();

            // ChunkyBacon is here!
            Pos pos = board->goku(player->me()).pos;

            costs[pos.i][pos.j] = 0;
            map[pos.i][pos.j] = Pos(-1, -1);

            pending.push(pos);

            while(not pending.empty())
            {
                Pos u = pending.front();
                pending.pop();

                if(board->cell(u).type == item)
                {
                    generate_path(u, path);
                    return true;
                }

                for(int i = Top; i <= Right; ++i)
                    push_adjacent(u, static_cast<Dir>(i));
            }

            return false;
        }
    };

    class Steering
    {
        PLAYER_NAME* player;

        Dir get_direction_to(const Pos &d)
        {
            Pos u = player->gme.pos;

            for(int i = Top; i <= Right; ++i)
            {
                Dir dir = static_cast<Dir>(i);

                if(u + dir == d)
                    return dir;
            }

            return None;
        }

    public:
        Steering(PLAYER_NAME* player)
        {
            this->player = player;
        }

        void follow_path(Path &path)
        {
            if(not path.empty())
            {
                Pos d = path.front();
                path.pop_front();

                player->move(get_direction_to(d));
            }
        }
    };

    Goku gme;
    Steering* steering;
    Dijkstra* compass;

    State* global_state;
    SType current_state;
    SType previous_state;
    vector<State*> states;

    bool has_turn()
    {
        return (gme.alive and (has_kinton(gme.type) or round() % 2 == 0));
    }

    bool is_strength_lower(double s)
    {
        return (s >= double(gme.strength) / double(max_strength()));
    }

    bool is_stronger_than(const Goku &g)
    {
        double a = double(1 + gme.strength);
        double b = double(2 + gme.strength + g.strength);

        return (a/b >= 0.8);
    }

    void analyze_environment()
    {
        
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

        // Initialize members of the class
        if(round() == 0)
        {
            compass = new Dijkstra(this);
            steering = new Steering(this);
            states = vector<State*>(4);
            states[CollectBalls] = new CollectBallsState();
            states[RecoverEnergy] = new RecoverEnergyState();

            global_state = new GlobalPlayerState();
            current_state = CollectBalls;
            states[current_state]->enter(this);
        }

        // Because it is not possible to analyze consequences of actions until next
        // round, it is necessary to process state transitions before execution
        states[current_state]->transition(this);

        // Analyze the current map for the current state
        analyze_environment();

        // This is the global state, it is used to put the player in some states when
        // they are prioritary (runaway, for example)
        global_state->execute(this);

        if(has_turn())
            states[current_state]->execute(this);
    }

    class GlobalPlayerState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            if(not player->is_in_state(RecoverEnergy) and player->is_strength_lower(0.1))
                player->change_state(RecoverEnergy);
        }
    };

    class CollectBallsState : public State
    {
        virtual void execute(PLAYER_NAME* player)
        {
            Path kpath;

            if(not has_kinton(player->gme.type))
                player->compass->search(Kinton, kpath);
            else if(player->is_strength_lower(0.5))
                player->compass->search(Bean, kpath);

            Path opath;

            if(has_ball(player->gme.type))
                player->compass->search(Capsule, opath);
            else
                player->compass->search(Ball, opath);

            if(opath.size() != 0 and (kpath.size() == 0 or opath.size() < kpath.size()))
                player->steering->follow_path(opath);
            else if(kpath.size() != 0)
                player->steering->follow_path(kpath);
            else
            {
                player->compass->search(Kinton, opath);
                player->steering->follow_path(opath);       
            }

        }
    };

    class RecoverEnergyState : public State
    {
        virtual void transition(PLAYER_NAME* player)
        {
            if(not player->is_strength_lower(0.1))
                player->change_state(CollectBalls);
        }

        virtual void execute(PLAYER_NAME* player)
        {
            Path bpath;

            player->compass->search(Bean, bpath);
            player->steering->follow_path(bpath);
        }
    };
    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
