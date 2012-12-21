#include "Player.hh"
#include <limits>
#include <vector>
#include <queue>

using namespace std;

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME                 Thor
#define PLAYER_NAME_STRING          "Thor"

#define ROUNDS_PER_BALL             30.0
#define PLAYER_SAFE_RADIUS          3
#define OBJECTIVE_SAFE_RADIUS       8
#define KAME_MIN_PROB               0
#define MAX_COLLISION_WAIT          3

#define NUM_MAPS                    3
#define MAP_DEFAULT                 0
#define MAP_ZIGZAG                  1
#define MAP_JBOSCH1                 2

#define NUM_PRIORITIES              9
#define NORMAL_KINTON               0
#define NORMAL_BEAN                 1
#define WITH_BALL_BEAN              2
#define KINTON_NORMAL_BEAN          3
#define KINTON_NORMAL_KINTON        4
#define KINTON_WITH_BALL_BEAN       5
#define KINTON_WITH_BALL_KINTON     6
#define KINTON_BEAN_KAME_MOD        7
#define KINTON_BETTER_MOD           8

/**
 * Player struct!
 */
struct PLAYER_NAME : public Player {

    /**
     * Player instance pointer. Useful to find the player
     * instance from the "outside". Like a singleton.
     */
    static PLAYER_NAME* inst;

    /**
     * Player priorities for maps:
     * 0: DEFAULT
     * 1: ZIGZAG
     * 2: JBOSCH1
     */
    static double priorities[NUM_MAPS][NUM_PRIORITIES];

    /**
     * Represents a table of costs for each (x,y) position.
     * It is updated in every radar/path search.
     */
    typedef vector< vector<int> > CostTable;

    /**
     * Represents a table of previous positions for each (x,y) position.
     * It is updated in every path search.
     */
    typedef vector< vector<Pos> > PrevPosTable;

    /**
     * Represents a cell which has a temporary item (an item that
     * regenerates).
     */
    struct TempCell
    {
        /**
         * Id of the temporary item.
         */
        int id;

        /**
         * Item type.
         */
        CType type;

        /**
         * Default TempCell constructor
         * @return A new empty TempCell
         */
        inline TempCell ()
        :   id(-1),
            type(Empty)
        {   }
    };

    /**
     * Represents the TempCells that a map contains.
     */
    typedef vector< vector<TempCell> > TempMap;

    /**
     * Represents a path in the board.
     */
    struct Path {
        /**
         * Type of the objective of the path.
         */
        CType otype;

        /**
         * First position in the path.
         */
        Pos first;

        /**
         * Last position in the path.
         */
        Pos end;

        /**
         * Travel time of the path (rounds to follow the path).
         */
        int trounds;

        /**
         * Rounds to wait (TempCell).
         */
        int wrounds;

        /**
         * Total cost of the path.
         */
        int cost;

        /**
         * Size of the path (number of cells).
         */
        int size;

        /**
         * A ghost path is a path which objective isnt available at
         * the current round.
         */
        bool ghost;

        /**
         * Temporary item id of the objective.
         */
        int temp_id;


        /**
         * Kinton time left after following the path.
         */
        int kinton;

        /**
         * Default constructor (empty path).
         */
        inline Path ()
        :   otype(Empty),
            first(-1, -1),
            end(-1, -1),
            trounds(0),
            wrounds(0),
            cost(0),
            size(0),
            kinton(0)
        {   }

        /**
         * Creates a path with an objective.
         * @param  type Type of the objective
         * @return      Path with the objective type given
         */
        inline Path (CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            trounds(0),
            wrounds(0),
            cost(0),
            size(0),
            kinton(0)
        {
            
        }

        /**
         * Path that represents the current position of a player.
         * @param  g Goku of a player
         * @return   Path for the goku g.
         */
        inline Path(const Goku &g)
        :   otype(Empty),
            trounds(0),
            wrounds(0),
            cost(0),
            size(0)
        {
            first = end = g.pos;

            if(has_kinton(g.type))
                kinton = g.kinton;
            else
                kinton = 0;
        }

        /**
         * Creates a path for the given player and uses it to
         * found a path to an item of the given type.
         * @param  g    Goku of a player
         * @param  type Type of the objective
         * @return      An empty path if no path is found,
         *                 otherwise returns the best path found.
         */
        inline Path(const Goku &g, CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            trounds(0),
            wrounds(0),
            cost(0),
            size(0),
            kinton(0)
        {
            Path root(g);

            PLAYER_NAME::inst->search(root, *this);
        }

        /**
         * Uses the given path as root to find a new
         * path to an item of the given type.
         * @param  p    A path
         * @param  type Type of the objective
         * @return      An empty path if no path is found,
         *                 otherwise returns the best path found.
         */
        inline Path(const Path &p, CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            trounds(0),
            wrounds(0),
            cost(0),
            size(0),
            kinton(0)
        {
            PLAYER_NAME::inst->search(p, *this);
        }

        /**
         * Tells whether the path is empty or not.
         * @return True if path is empty, false otherwise.
         */
        bool empty() const
        {
            return (size == 0);
        }

        /**
         * Sets the size of the current path, calculating
         * costs given a root path.
         * @param root A root path
         * @param s    Size to set
         */
        void set_size(const Path &root, int s)
        {
            size = s;
            wrounds = 0;

            if(root.kinton >= size)
            {
                kinton += root.kinton - size;
                trounds = size + root.cost;
            }
            else
            {
                kinton = 0;
                trounds = (size * 2) + root.cost - root.kinton;

                if(trounds % 2 == 0)
                    trounds += 1;
            }
        }
    };

    /**
     * Factory: returns a new instance of this class.
     * Do not modify this function.
     */
    static Player* factory () {
        return new PLAYER_NAME;
    }

    /**
     * The goku of the player.
     */
    Goku gme;

    /**
     * Map of TempCells.
     */
    TempMap map;

    /**
     * Map where the player is fighting.
     */
    int map_id;

    /**
     * Counter of rounds to not wait forever when avoiding
     * or throwing kames.
     */
    int wait_round;

    /**
     * Path that the player must follow.
     */
    Path path;

    /**
     * Costs for every position in the last radar or searched path.
     */
    CostTable costs;

    /**
     * Previous positions for every position in the last radar or searched path.
     */
    PrevPosTable prev_pos;

    /**
     * Represents a queue of the pending positions to visit in a search.
     * It is updated in every radar/path search.
     */
    queue<Pos> pending;

    /**
     * Searches for a path in the board.
     * @param  root Path from which calculate the new one
     * @param  path Path with objective where to store the found path
     * @return      True if a path is found, false otherwise
     */
    bool search(const Path &root, Path &path)
    {
        path.cost = maxint;
        costs = CostTable(rows(), vector<int>(cols(), maxint));

        prev_pos = PrevPosTable(rows(), vector<Pos>(cols()));
        pending = queue<Pos>();

        Pos p = root.first;
        costs[p.i][p.j] = 0;
        pending.push(p);

        Path current(path.otype);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            current.set_size(root, costs[u.i][u.j]);

            if(found(u, current))
            {
                if(current.ghost)
                    set_wrounds(current, map[u.i][u.j].id);

                current.end = u;
                generate_path(current);

                if(is_better(current, path))
                    path = current;
            }

            if(current.trounds > path.cost)
                break;

            for(int i = Top; i <= Right; ++i)
            {
                Pos v = u + static_cast<Dir>(i);

                if(should_push(u, v))
                {
                    prev_pos[v.i][v.j] = u;
                    pending.push(v);   
                }
            }
        }

        return (not path.empty());
    }

    bool found(const Pos &u, Path &path)
    {
        if(cell(u).type == path.otype)
        {
            path.ghost = false;
            return true;
        }

        if(map[u.i][u.j].type == path.otype)
        {
            path.ghost = true;
            return true;
        }

        return false;
    }

    void set_wrounds(Path &path, int item)
    {
        int rtime;

        if(path.otype == Bean)
            rtime = (beans()[item]).time;
        else
            rtime = (kintons()[item]).time;

        path.wrounds = rtime - path.trounds;

        if(path.kinton == 0 and (round() + path.wrounds) % 2 != 0)
            path.wrounds += 1;

        path.temp_id = item;
    }

    void calculate_cost(Path &path)
    {
        if(path.otype == Kinton)
            path.kinton += kinton_life_time();

        path.cost = path.trounds;

        if(path.wrounds > 0)
            path.cost += path.wrounds;
    }

    int abs(int x)
    {
        if(x < 0)
            return -x;

        return x;
    }

    bool is_better(const Path &current, const Path &best)
    {
        if(best.empty())
            return true;

        return (abs(current.wrounds) < abs(best.wrounds));
    }

    /**
     * Detects gokus inside a radius r from position p.
     * @param  p     Position where to start the radar.
     * @param  r     Radius to search.
     * @param  gokus Empty vector to store gokus found.
     * @return       True if one or more gokus are found,
     *                    false otherwise.
     */
    bool radar(const Pos &p, int r, vector<bool> &gokus)
    {
        bool detected = false;

        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));
        pending = queue<Pos>();

        costs[p.i][p.j] = 0;
        pending.push(p);

        // If gokus vector is not initialized, then init to false!
        if(gokus.size() == 0)
            gokus = vector<bool>(nb_players(), false);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(costs[u.i][u.j] > r)
                break;

            int gid = cell(u).id;

            if(gid >= 0)
            {
                gokus[gid] = true;
                detected = true;
            }

            for(int i = Top; i <= Right; ++i)
            {
                Pos v = u + static_cast<Dir>(i);

                if(should_push(u, v))
                    pending.push(v);
            }
        }

        return detected;
    }

    /**
     * Tells whether an adjacent position should be pushed to
     * the pending queue.
     * @param u   Reference position
     * @param a   Adjacent position to u
     */
    bool should_push(const Pos &u, const Pos &a)
    {
        if(not pos_ok(a))
            return false;

        if(cell(a).type == Rock)
            return false;

        int new_cost = 1 + costs[u.i][u.j];

        if(new_cost < costs[a.i][a.j])
        {
            costs[a.i][a.j] = new_cost;

            return true;
        }

        return false;
    }

    /**
     * Generates a path from the last position.
     * @param p Empty path where to update data.
     */
    void generate_path(Path &path)
    {
        Pos u = path.end;

        for(int i = 1; i <= path.size; ++i)
        {
            int gid = cell(u).id;

            /**
             * Avoids gokus. Prototype, it does not work properly sometimes.
             */
            if(gid >= 0 and not is_ally(gid))
            {
                double pfight = prob_win_fight(goku(gid));
                
                if(pfight < 0.75)
                {
                    int extra_rounds = double(goku_regen_time()) + 10.0 * pfight;

                    if(path.wrounds < 0)
                        path.wrounds -= extra_rounds;
                    else
                        path.wrounds += extra_rounds;

                    costs[path.end.i][path.end.j] += extra_rounds;
                }
            }

            if(i < path.size)
                u = prev_pos[u.i][u.j];
        }
        
        path.first = u;

        calculate_cost(path);
    }

    /**
     * Gets the best kame direction using a simple score system.
     * @return The best kame direction, None if there are no gokus
     *             aligned.
     */
    Dir best_kame_dir(double &score)
    {
        Dir best_dir = None;
        double potential_balls = double(nb_rounds() - round()) / ROUNDS_PER_BALL;
        score = double(gme.balls) - potential_balls + strength_prop(gme);

        vector<bool> gokus;
        radar(path.end, OBJECTIVE_SAFE_RADIUS, gokus);
        radar(gme.pos, PLAYER_SAFE_RADIUS, gokus);

        for(int i = Top; i <= Right; ++i)
        {
            double dir = dir_score(static_cast<Dir>(i), gokus);
            
            if(dir > 0 and dir > score)
            {
                best_dir = static_cast<Dir>(i);
                score = dir;
            }
        }

        return best_dir;
    }

    /**
     * Updates the best kame direction to another if its better.
     * @param d           New direction
     * @param radar_gokus Gokus inside dangerous areas
     */
    double dir_score(const Dir &d, const vector<bool> &radar_gokus)
    {
        double score = 0;

        Pos u = gme.pos + d;

        while(cell(u).type != Rock)
        {
            int gid = cell(u).id;

            if(gid >= 0)
                score += goku_score(gid, radar_gokus[gid]);

            u += d;
        }

        return score;
    }

    /**
     * Returns the score of a kame direction for a goku found.
     * @param goku_id Id of the goku found
     * @param inside  Whether the goku is inside dangerous areas or not.
     * @return tTe score of a kame direction for a goku found.
     */
    double goku_score(int goku_id, bool inside)
    {
        if(is_ally(goku_id))
            return 0;

        Goku g = goku(goku_id);

        double g_score = double(g.balls) + strength_prop(g);

        if(has_ball(g.type))
            g_score *= 3.0;

        if(g.strength > kamehame_penalty())
            g_score *= 3.0;

        if(inside)
            g_score *= 2.0;
        
        return g_score;
    }

    /**
     * Gets the direction to follow to move to the adjacent
     * position d.
     * @param  d Adjacent position where to go.
     * @return   Direction to follow. None if position is not
     *           adjacent.
     */
    Dir get_direction_to(const Pos &d)
    {
        Pos u = gme.pos;

        for(int i = Top; i <= Right; ++i)
        {
            Dir dir = static_cast<Dir>(i);

            if(u + dir == d)
                return dir;
        }

        return None;
    }

    /**
     * Sets a path be followed by the player.
     * @param path The path to set
     */
    void set_path(const Path &path)
    {
        this->path = path;
    }

    /**
     * Tells whether the player has turn or not.
     * @return True if the player has turn, false otherwise.
     */
    bool has_turn()
    {
        return (gme.alive and (has_kinton(gme.type) or round() % 2 == 0));
    }

    double prob_win_fight(const Goku &g)
    {
        double a = double(1 + gme.strength);
        double b = double(2 + gme.strength + g.strength);

        return a/b;
    }

    bool is_ally(int gid)
    {
        string gname = name(gid);

        return (gname == PLAYER_NAME_STRING or gname == "UltimateCR7");
    }

    /**
     * Returns the current strenght proportion of the player.
     * @return The strength proportion of the player.
     */
    double strength_prop(const Goku &g)
    {
        return double(g.strength) / double(max_strength());
    }

    /**
     * Returns the life proportion of the current kinton.
     * @return The life proportion of the current kinton.
     */
    double kinton_life_prop()
    {
        return double(gme.kinton) / double(kinton_life_time());
    }

    /**
     * Returns the player probability to throw a kamehame.
     * @return The probability to throw a kamehame.
     */
    double prob_kame(const Goku &g)
    {
        return
            double(1 + g.strength - kamehame_penalty()) /
            double(1 + max_strength() - kamehame_penalty());
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
        if(round() == 0)
        {
            inst = this;
            wait_round = 0;

            detect_map();
            analyze_map();
        }

        gme = goku(me()); // Player goku shortcut
        path = Path(); // Clear the current path

        if(has_turn())
        {
            collect_balls();

            Dir kame;

            if(path.empty())
                wander();

            if(objectives_detected(kame))
                throw_kamehame(kame);

            else
            {
                if(should_wait() or threat_detected())
                    move(None);

                else
                {
                    move(get_direction_to(path.first));
                    wait_round = round();
                }
            }
        }
    }

    bool should_wait()
    {
        if(not path.ghost)
            return false;

        if(get_direction_to(path.end) == None)
            return false;

        int rtime;

        if(path.otype == Bean)
            rtime = beans()[path.temp_id].time;
        else
            rtime = kintons()[path.temp_id].time;

        if(rtime <= 0)
            return false;

        return true;
    }

    vector<Dir> complement(Dir d)
    {
        vector<Dir> c(2);

        switch(d)
        {
            case Top:
            case Bottom:
                c[0] = Left;
                c[1] = Right;
                break;

            default:
                c[0] = Top;
                c[1] = Bottom;
                break;
        }

        return c;
    }

    bool threat_detected()
    {
        if(round() - wait_round > goku_regen_time())
            return false;

        vector<Dir> d = complement(get_direction_to(path.first));

        for(int i = 0; i < 2; ++i)
        {
            Pos u = path.first;

            while(cell(u).type != Rock)
            {
                int gid = cell(u).id;

                if(gid >= 0 and not is_ally(gid) and status(gid) != 1.0)
                {   
                    Goku g = goku(gid);

                    if(prob_kame(g) > 0.3)
                        return true;
                }

                u += d[i];
            }
        }

        return false;
    }

    void detect_map()
    {
        if(rows() == 15 and cols() == 31 and nb_kintons() == 2)
            map_id = MAP_ZIGZAG;

        else if(nb_beans() == 8)
            map_id = MAP_JBOSCH1;

        else
            map_id = MAP_DEFAULT;

        cout << map_id << endl;
    }

    void analyze_map()
    {
        map = TempMap(rows(), vector<TempCell>(cols()));

        for(int i = 0; i < rows(); ++i)
        {
            for(int j = 0; j < cols(); ++j)
            {
                CType cell_type = cell(i, j).type;

                if(cell_type != Bean and cell_type != Kinton)
                    continue;

                int id;

                if(cell_type == Bean)
                    id = find_id(Pos(i, j), cell_type);
                else
                    id = find_id(Pos(i, j), cell_type);

                map[i][j].id = id;
                map[i][j].type = cell_type;
            }
        }
    }

    int find_id(const Pos &p, CType type)
    {
        if(type == Bean)
            return find_id(p, beans());
        
        return find_id(p, kintons());
    }

    int find_id(const Pos &p, const vector<Magic_Bean> &beans)
    {
        for(int i = 0; i < nb_beans(); ++i)
            if(beans[i].pos == p)
                return i;

        return -1;
    }

    int find_id(const Pos &p, const vector<Kinton_Cloud> &kintons)
    {
        for(int i = 0; i < nb_kintons(); ++i)
            if(kintons[i].pos == p)
                return i;

        return -1;
    }

    /**
     * Makes the goku collect balls!
     */
    void collect_balls()
    {
        switch(gme.type)
        {
            case With_Ball:
                collect_balls_with_ball();
                break;

            case On_Kinton:
                collect_balls_on_kinton();
                break;

            case On_Kinton_With_Ball:
                collect_balls_on_kinton_with_ball();
                break;

            default:
                collect_balls_normal();
                break;
        }
    }

    Path choose_path(CType a, CType b, double h)
    {
        Path p1(gme, a);
        Path p2(gme, b);

        if(p1.empty())
            return p2;

        if(p2.empty())
            return p1;

        Path alt(p1, b);

        if(alt.cost * h <= p2.cost)
            return p1;
        else
            return p2;
    }

    bool is_kinton_awesome_for(Path &kin, Path &obj)
    {
        if(kin.empty())
            return false;

        if(obj.empty())
            return true;

        Path kin_obj(kin, obj.otype);

        return (kin_obj.cost * priorities[map_id][NORMAL_KINTON] <= 1.5 * double(obj.cost));
    }

    bool is_better_on_kinton(Path &alt, Path &obj, double m)
    {
        if(alt.empty())
            return false;

        if(obj.empty())
            return true;

        Path alt_obj(alt, obj.otype);

        return (alt_obj.cost * m <= obj.cost);
    }

    void go_on_foot(CType obj, double priority)
    {
        Path kinton(gme, Kinton);
        Path bean_obj = choose_path(Bean, obj, strength_prop(gme) * priority);

        if(is_kinton_awesome_for(kinton, bean_obj))
            set_path(kinton);
        else
            set_path(bean_obj);
    }

    void go_on_kinton(CType obj, double pbean, double pkinton)
    {
        Path kinton(gme, Kinton);
        Path bean_obj;

        double heuristic = strength_prop(gme);

        if(not has_ball(gme.type) and kamehame_penalty() > gme.strength)
            heuristic /= priorities[map_id][KINTON_BEAN_KAME_MOD];

        bean_obj = choose_path(Bean, obj, heuristic * pbean);

        heuristic = kinton_life_prop() + priorities[map_id][KINTON_BETTER_MOD];
        if(is_better_on_kinton(kinton, bean_obj, heuristic * pkinton))
            set_path(kinton);
        else
            set_path(bean_obj);
    }

    void collect_balls_normal()
    {
        go_on_foot(Ball, priorities[map_id][NORMAL_BEAN]);
    }

    void collect_balls_with_ball()
    {
        go_on_foot(Capsule, priorities[map_id][WITH_BALL_BEAN]);
    }

    void collect_balls_on_kinton()
    {
        go_on_kinton(Ball, priorities[map_id][KINTON_NORMAL_BEAN],
            priorities[map_id][KINTON_NORMAL_KINTON]);
    }

    void collect_balls_on_kinton_with_ball()
    {
        go_on_kinton(Capsule, priorities[map_id][KINTON_WITH_BALL_BEAN],
            priorities[map_id][KINTON_WITH_BALL_KINTON]);
    }

    /**
     * Makes the player wander.
     */
    void wander()
    {
        Dir a[] = {Left, Right, Bottom, Top};
        Pos u;

        do {
            Dir dir = a[randomize() % 4];
            u = gme.pos + dir;
        } while(not pos_ok(u) or cell(u).type == Rock);

        path.first = u;
        path.end = u;
        path.size = 1;
    }

    /**
     * Tells whether threats have been detected and the player can do
     * 'something' about it.
     * @param  kame Best direction to fix the threats. Youknowwhatimean :3
     * @return      True if there are threats detected and the player can
     *                   'fix' them, false otherwise.
     */
    bool objectives_detected(Dir &kame)
    {
        if(prob_kame(gme) < KAME_MIN_PROB)
            return kame_to_front(kame);

        double score;
        kame = best_kame_dir(score);

        return (kame != None);
    }

    bool kame_to_front(Dir &kame)
    {
        if(round() - wait_round > goku_regen_time())
            return false;

        kame = get_direction_to(path.first);
        int gid = cell(gme.pos + kame).id;

        if(gid < 0)
            return false;

        if(is_ally(gid))
            return false;

        return (prob_win_fight(goku(gid)) < 0.75);
    }
    
};

PLAYER_NAME* PLAYER_NAME::inst = NULL;

double PLAYER_NAME::priorities[NUM_MAPS][NUM_PRIORITIES] = {
    {1.0, 1.0, 1.5, 1.5, 0.8, 1.5, 1.0, 2.0, 0.0}, // DEFAULT
    {1.0, 1.0, 4.0, 4.0, 4.0, 4.0, 5.0, 1.0, 0.1}, // ZIGZAG
    {1.0, 1.0, 1.5, 2.5, 0.5, 3.0, 1.0, 1.0, 0.0}  // JBOSCH1
};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
