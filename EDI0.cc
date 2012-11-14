#include "Player.hh"
#include <list>
#include <queue>
#include <limits>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME EDI0

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
    typedef pair<int, Pos> Priority;
    typedef priority_queue< Priority, vector<Priority>, greater<Priority> > IndexedQueue;

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
        GetBall, GetBean, GetKinton, GoToCapsule
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

            int cost;

            if(strong_goku_at(v))
                cost = 20;
            else
                cost = 1;
            
            int new_cost = cost + costs[u.i][u.j];

            if(new_cost < costs[v.i][v.j])
            {
                costs[v.i][v.j] = new_cost;
                map[v.i][v.j] = u;
                pending.push(make_pair(new_cost, v));
            }
        }

        bool strong_goku_at(const Pos &u)
        {
            int nplayers = board->nb_players();

            for(int i = 0; i < nplayers; ++i)
            {
                if(i != player->me())
                {
                    Goku g = board->goku(i);

                    if(g.pos == u and player->is_stronger_than(g))
                        return true;
                }
            }

            return false;
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

            pending.push(make_pair(0, pos));

            while(not pending.empty())
            {
                Pos u = pending.top().second;
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
        Path path;
        Dijkstra* dijkstra;
        CType objective;

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
            dijkstra = new Dijkstra(player);
        }

        int get_path_size()
        {
            return path.size();
        }

        void seek_to(CType obj)
        {
            objective = obj;
        }

        void find_path()
        {
            path.clear();
            dijkstra->search(objective, path);
        }

        void move()
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
    State* global_state;
    SType current_state;
    SType previous_state;
    vector<State*> states;


    bool has_turn()
    {
        return (gme.alive and (has_kinton(gme.type) or round() % 2 == 0));
    }

    bool has_strength_low()
    {
        if(has_kinton(gme.type))
            return (gme.strength <= (moving_penalty() * (steering->get_path_size() + 18)));

        return (gme.strength <= (moving_penalty() * (steering->get_path_size() + 10)));
    }

    bool is_too_far()
    {
        return (not has_kinton(gme.type) and steering->get_path_size() > 20);
    }

    bool is_stronger_than(const Goku &g)
    {
        double a = double(1 + gme.strength);
        double b = double(2 + + gme.strength + g.strength);

        return (a/b >= 0.8);
    }

    void analyze_environment()
    {
        steering->find_path();
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
            steering = new Steering(this);
            states = vector<State*>(4);
            states[GetBall] = new GetBallState();
            states[GetBean] = new GetBeanState();
            states[GetKinton] = new GetKintonState();
            states[GoToCapsule] = new GoToCapsuleState();

            global_state = new GlobalPlayerState();
            current_state = GetBall;
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
            if(not player->is_in_state(GetBean) and player->has_strength_low())
                player->change_state(GetBean);

            else if(not player->is_in_state(GetKinton) and player->is_too_far())
                player->change_state(GetKinton);

            else
            {
                if(player->steering->get_path_size() == 0)
                    player->change_state(has_kinton(player->gme.type) ? GetBean : GetKinton);
            }
        }
    };

    class GetBallState : public State
    {
        virtual void enter(PLAYER_NAME* player)
        {
            player->steering->seek_to(Ball);
        }

        virtual void transition(PLAYER_NAME* player)
        {
            if(has_ball(player->gme.type))
                player->change_state(GoToCapsule);
        }

        virtual void execute(PLAYER_NAME* player)
        {
            player->steering->move();
        }
    };

    class GoToCapsuleState : public State
    {
        virtual void enter(PLAYER_NAME* player)
        {
            player->steering->seek_to(Capsule);
        }

        virtual void transition(PLAYER_NAME* player)
        {
            if(! has_ball(player->gme.type))
                player->change_state(GetBall);
        }

        virtual void execute(PLAYER_NAME* player)
        {
            player->steering->move();
        }
    };

    class GetBeanState : public State
    {
        virtual void enter(PLAYER_NAME* player)
        {
            player->steering->seek_to(Bean);
            player->steering->find_path();
        }

        virtual void transition(PLAYER_NAME* player)
        {
            if(has_ball(player->gme.type))
                player->change_state(GoToCapsule);
            else
                player->change_state(GetBall);
        }

        virtual void execute(PLAYER_NAME* player)
        {
            player->steering->move();
        }
    };

    class GetKintonState : public State
    {
        virtual void enter(PLAYER_NAME* player)
        {
            player->steering->seek_to(Kinton);
            player->steering->find_path();
        }

        virtual void transition(PLAYER_NAME* player)
        {
            if(has_ball(player->gme.type))
                player->change_state(GoToCapsule);
            else
                player->change_state(GetBall);
        }

        virtual void execute(PLAYER_NAME* player)
        {
            player->steering->move();
        }
    };
    
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
