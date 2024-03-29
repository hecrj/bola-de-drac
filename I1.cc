#include "Player.hh"
#include <limits>
#include <vector>
#include <queue>

using namespace std;

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME I1
#define PLAYER_NAME_STRING "I1"

/**
 * Estimated minimum number of rounds for collecting a ball.
 */
#define ROUNDS_PER_BALL                30.0

/**
 * Dangerous area around the player.
 */
#define PLAYER_SAFE_RADIUS             5

 /**
  * Dangerous area around the objective.
  */
#define OBJECTIVE_SAFE_RADIUS          8

/**
 * Minimum probability to throw a kame.
 */
#define KAME_MIN_PROB                  0.75

/**
 * Priorities (smaller indicates more priority).
 */
#define PRIORITY_NORMAL_BEAN                1
#define PRIORITY_WITH_BALL_BEAN             1.5
#define PRIORITY_KINTON_NORMAL_BEAN         1
#define PRIORITY_KINTON_NORMAL_KINTON       0.5
#define PRIORITY_KINTON_WITH_BALL_BEAN      1.5
#define PRIORITY_KINTON_WITH_BALL_KINTON    1

/**
 * Priority modifiers
 */
#define PRIORITY_KINTON_BEAN_MOD       0.7
#define PRIORITY_KINTON_BEAN_KAME_MOD  2

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
     * Represents a table of costs for each (x,y) position.
     * It is updated in every radar/path search.
     */
    typedef vector< vector<int> > CostTable;

    /**
     * Represents a table of previous positions for each (x,y) position.
     * It is updated in every path search.
     */
    typedef vector< vector<Pos> > PrevPosTable;

    struct TempCell
    {
        int id;
        CType type;

        inline TempCell ()
        :   id(-1),
            type(Empty)
        {   }
    };

    typedef vector< vector<TempCell> > Map;

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
         * Size of the path (distance until end).
         */
        int size;

        /**
         * Default constructor (empty path).
         */
        inline Path ()
        :   otype(Empty),
            first(-1, -1),
            end(-1, -1),
            size(0)
        {   }

        /**
         * Constructor given an objective type.
         * @param  type Objective type.
         * @return      A new path with the given objective path.
         */
        inline Path (CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            size(0)
        {   }

        /**
         * Creates and searches for a path in the board.
         * @param  u    Search start position
         * @param  type Objective type
         * @return      A new path filled with info from the found path,
         *                empty if there is no path available.
         */
        inline Path(const Pos &u, CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            size(0)
        {
            PLAYER_NAME::inst->search(u, 0, *this);
        }

        inline Path(const Path &p, CType type)
        :   otype(type),
            first(-1, -1),
            end(-1, -1),
            size(0)
        {
            int relative_rounds = PLAYER_NAME::inst->calculate_rounds(p.size, 0);

            PLAYER_NAME::inst->search(p.end, relative_rounds, *this);
        }

        /**
         * Tells whether the path is empty or not.
         * @return True if path is empty, false otherwise.
         */
        bool empty()
        {
            return (size == 0);
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

    Map map;

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
     * @param  p    Position where to start the search
     * @param  path Path with objective where to store the found path
     * @return      True if a path is found, false otherwise
     */
    bool search(const Pos &p, int rel_rounds, Path &path)
    {
        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));
        prev_pos = PrevPosTable(rows(), vector<Pos>(cols()));
        pending = queue<Pos>();

        costs[p.i][p.j] = 0;
        pending.push(p);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(cell(u).type == path.otype)
            {
                generate_path(u, path);
                return true;
            }

            if(map[u.i][u.j].type == path.otype)
            {
                if(is_available_when_arrival(u, path.otype, rel_rounds))
                {
                    generate_path(u, path);
                    return true;
                }
            }

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

        return false;
    }

    bool is_available_when_arrival(const Pos &u, CType type, int rel_rounds)
    {
        if(type == Bean)
            return is_available_when_arrival(u, beans(), rel_rounds);
        
        return is_available_when_arrival(u, kintons(), rel_rounds);
    }

    bool is_available_when_arrival(const Pos &u, const vector<Magic_Bean> &beans, int rel_rounds)
    {
        int id = map[u.i][u.j].id;

        return (beans[id].time < calculate_rounds(costs[u.i][u.j], rel_rounds));
    }

    bool is_available_when_arrival(const Pos &u, const vector<Kinton_Cloud> &kintons, int rel_rounds)
    {
        int id = map[u.i][u.j].id;

        return (kintons[id].time < calculate_rounds(costs[u.i][u.j], rel_rounds));
    }

    int calculate_rounds(int cost, int rel_rounds)
    {
        int rounds = cost * 2;

        if(has_kinton(gme.type))
        {
            if(gme.kinton >= (rounds + rel_rounds))
                return cost;
            
            return rounds + rel_rounds - gme.kinton;
        }

        return rounds + rel_rounds;
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
     * Generates a path given the last position and
     * an updated previous position table.
     * @param u The last position of the path.
     * @param p Empty path where to update data.
     */
    void generate_path(Pos u, Path &p)
    {
        p.size = costs[u.i][u.j];
        p.end = u;

        for(int i = 1; i < p.size; ++i)
            u = prev_pos[u.i][u.j];
        
        p.first = u;
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
        radar(path.end, 8, gokus);
        radar(gme.pos, 3, gokus);

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
     * Updates the score of a kame direction for a goku found.
     * @param goku_id Id of the goku found
     * @param inside  Whether the goku is inside dangerous areas or not.
     */
    double goku_score(int goku_id, bool inside)
    {
        if(name(goku_id) == PLAYER_NAME_STRING)
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

    /**
     * Tells whether the player is stronger than g.
     * @param  g Goku rival.
     * @return   True if there is a considerable probability
     *                to win a fight versus g, false otherwise.
     */
    bool is_stronger_than(const Goku &g)
    {
        double a = double(1 + gme.strength);
        double b = double(2 + gme.strength + g.strength);

        return (a/b >= 0.8);
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
    double prob_kame()
    {
        return
            double(1 + gme.strength - kamehame_penalty()) /
            double(1 + max_strength() - kamehame_penalty());
    }

    /**
     * Tells whether a path to a kinton kin is awesome for
     * an objective path. 
     * @param  kin Path to a kinton
     * @param  obj Path to an objective
     * @return     True if kinton is awesome!
     *                  False otherwise D:
     */
    bool is_kinton_awesome_for(Path &kin, Path &obj)
    {
        if(kin.empty())
            return false;

        if(obj.empty())
            return true;

        Path kin_obj(kin, obj.otype);
        double heuristic = double(kin.size) + (double(kin_obj.size) / 2.0);

        return (heuristic <= 2.0 * double(obj.size));
    }

    /**
     * Tells whether is better to go to an alternative path
     * than go to the objective directly, when on kinton.
     * @param  alt Alternative path to test
     * @param  obj Objective path
     * @param  m   Some external heuristic. The greater the less probability
     *             to choose the alternative path.
     * @return     True if the alternative path is better, false otherwise.
     */
    bool is_better_on_kinton(Path &alt, Path &obj, double m)
    {
        if(alt.empty())
            return false;

        if(obj.empty())
            return true;

        Path alt_obj(alt, obj.otype);
        double heuristic = double(alt.size + alt_obj.size) / 2.0;

        return (heuristic <= (1.5 - m) * 1.5 * double(obj.size) / 2.0);
    }

    /**
     * Tells whether is better to go to an alternative path
     * than go to the objective directly.
     * @param  alt Alternative path to test
     * @param  obj Objective path
     * @param  m   Some external heuristic. The greater the less probability
     *             to choose the alternative path.
     * @return     True if the alternative path is better, false otherwise.
     */
    bool is_better(Path &alt, Path &obj, double m)
    {
        if(alt.empty())
            return false;

        if(obj.empty())
            return true;

        Path alt_obj(alt, obj.otype);
        double heuristic = double(alt.size + alt_obj.size);

        return (heuristic * m <= double(obj.size) * (2.0 - m));
    }

    Path choose_path(CType a, CType b, double h)
    {
        Path p1(gme.pos, a);
        Path p2(gme.pos, b);

        if(has_kinton(gme.type))
        {
            if(is_better_on_kinton(p1, p2, h))
                return p1;
            
            return p2;
        }
        
        if(is_better(p1, p2, h))
            return p1;
        
        return p2;
    }

    void go_on_foot(CType obj, double priority)
    {
        Path kinton(gme.pos, Kinton);
        Path bean_obj = choose_path(Bean, obj, strength_prop(gme) * priority);

        if(is_kinton_awesome_for(kinton, bean_obj))
            set_path(kinton);
        else
            set_path(bean_obj);
    }

    void go_on_kinton(CType obj, double pbean, double pkinton)
    {
        Path kinton(gme.pos, Kinton);
        Path bean_obj;

        double heuristic = strength_prop(gme) / (2.0 - PRIORITY_KINTON_BEAN_MOD);

        if(kamehame_penalty() > gme.strength)
            heuristic /= PRIORITY_KINTON_BEAN_KAME_MOD;

        bean_obj = choose_path(Bean, obj, heuristic * pbean);

        if(is_better_on_kinton(kinton, bean_obj, kinton_life_prop() * pkinton))
            set_path(kinton);
        else
            set_path(bean_obj);
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
                move(get_direction_to(path.first));
        }
    }

    void analyze_map()
    {
        map = Map(rows(), vector<TempCell>(cols()));

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

    void collect_balls_normal()
    {
        go_on_foot(Ball, PRIORITY_NORMAL_BEAN);
    }

    void collect_balls_with_ball()
    {
        go_on_foot(Capsule, PRIORITY_WITH_BALL_BEAN);
    }

    void collect_balls_on_kinton()
    {
        go_on_kinton(Ball, PRIORITY_KINTON_NORMAL_BEAN, PRIORITY_KINTON_NORMAL_KINTON);
    }

    void collect_balls_on_kinton_with_ball()
    {
        go_on_kinton(Capsule, PRIORITY_KINTON_WITH_BALL_BEAN, PRIORITY_KINTON_WITH_BALL_KINTON);
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
        if(kamehame_penalty() > gme.strength)
            return false;

        if(prob_kame() < KAME_MIN_PROB)
            return kame_to_front(kame);

        double score;
        kame = best_kame_dir(score);

        return (kame != None);
    }

    bool kame_to_front(Dir &kame)
    {
        kame = get_direction_to(path.first);
        int gid = cell(gme.pos + kame).id;

        if(gid < 0)
            return false;

        return (not is_stronger_than(goku(gid)));
    }
    
};

PLAYER_NAME* PLAYER_NAME::inst = NULL;


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
