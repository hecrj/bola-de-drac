#include "Player.hh"
#include <limits>
#include <queue>
#include <cmath>

using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Meteor1

struct PLAYER_NAME : public Player {

    static PLAYER_NAME* inst; // Player instance pointer

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
    typedef queue<Pos> IndexedQueue;

    struct Path {
        CType type;
        Pos next;
        Pos end;
        int size;

        /**
         * Default constructor (empty path).
         */
        inline Path ()
        :   type(Empty),
            next(-1, -1),
            end(-1, -1),
            size(0)
        {   }

        /**
         * Given constructor.
         */
        inline Path (CType type)
        :   type(type),
            next(-1, -1),
            end(-1, -1),
            size(0)
        {   }

        inline Path(const Pos &u, CType type)
        :   type(type),
            next(-1, -1),
            end(-1, -1),
            size(0)
        {
            PLAYER_NAME::inst->search(u, *this);
        }

        bool empty()
        {
            return size == 0;
        }
    };

    Goku gme;
    Path path;

    CostTable costs;
    NodeTable map;
    IndexedQueue pending;

    void generate_path(Pos u, Path &p)
    {
        p.size = costs[u.i][u.j];
        p.end = u;

        for(int i = 1; i < p.size; ++i)
            u = map[u.i][u.j];
        
        p.next = u;
    }

    void push_adjacent(const Pos &u, Dir dir)
    {
        Pos v = u + dir;

        if(not pos_ok(v))
            return;

        if(cell(v).type == Rock)
            return;

        int new_cost = 1 + costs[u.i][u.j];

        if(new_cost < costs[v.i][v.j])
        {
            costs[v.i][v.j] = new_cost;
            map[v.i][v.j] = u;
            pending.push(v);
        }
    }

    bool search(const Pos &p, Path &path)
    {
        // Minimum distances to each Cell
        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));

        // Previous position to make paths
        map = NodeTable(rows(), vector<Pos>(cols()));

        // Pending positions to visit
        pending = IndexedQueue();

        costs[p.i][p.j] = 0;

        pending.push(p);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(cell(u).type == path.type)
            {
                generate_path(u, path);
                return true;
            }

            for(int i = Top; i <= Right; ++i)
                push_adjacent(u, static_cast<Dir>(i));
        }

        return false;
    }

    struct GInfo {
        int id;
        int distance;
    };

    int get_goku_id_in(const Pos &u)
    {
        int players = nb_players();

        for(int i = 0; i < players; ++i)
        {
            if(i != me())
            {
                if(goku(i).alive and goku(i).pos == u)
                    return i;
            }
        }

        return -1;
    }

    bool is_aligned(const Pos &u, const Dir &d)
    {
        Pos n = gme.pos + d;

        while(pos_ok(n) and cell(n).type != Rock)
        {
            if(n == u)
                return true;

            n += d;
        }

        return false;
    }

    Dir get_aligned_dir(int gid)
    {
        Dir d[] = {Top, Right, Bottom, Left};

        Pos g = goku(gid).pos;

        if(gme.pos.i != g.i and gme.pos.j != g.j)
            return None;

        for(int i = 0; i < 4; ++i)
            if(is_aligned(g, d[i]))
                return d[i];

        return None;
    }

    int get_max_index(const vector<int> &v)
    {
        int max = 0;
        int n = v.size();

        for(int i = 0; i < n; ++i)
        {
            if(v[max] < v[i])
                max = i;
        }

        return max;
    }

    bool radar(const Pos &u, int r, vector<bool> &gokus)
    {
        bool detected = false;

        // Minimum distances to each cell
        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));

        // Pending positions to visit
        pending = IndexedQueue();

        gokus = vector<bool>(nb_players(), false);

        costs[u.i][u.j] = 0;

        pending.push(u);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(costs[u.i][u.j] > r)
                break;

            int gid = get_goku_id_in(u);

            if(gid >= 0)
            {
                gokus[gid] = true;
                detected = true;
            }

            for(int i = Top; i <= Right; ++i)
                push_adjacent(u, static_cast<Dir>(i));
        }

        return detected;
    }

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

    void set_path(const Path &path)
    {
        this->path = path;
    }

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

    double prob_kame()
    {
        return
            double(1 + gme.strength - kamehame_penalty()) /
            double(1 + max_strength() - kamehame_penalty());
    }

    struct KameDir
    {
        Dir dir;
        double score_in;
        double score_out;

        inline KameDir()
        :   dir(None),
            score_in(0.0),
            score_out(0.0)
        { }
    };

    void kame_dir_update(const Dir &d, const vector<bool> &radar_gokus, KameDir &kame)
    {
        KameDir current;
        current.dir = d;

        Pos u = gme.pos + d;

        while(cell(u).type != Rock)
        {
            int gid = cell(u).id;

            if(gid >= 0)
            {
                Goku g = goku(gid);
                double g_score = double(g.balls) + (double(g.strength) / double(max_strength()));

                if(has_ball(g.type))
                    g_score *= 3.0;

                if(g.strength > kamehame_penalty())
                    g_score *= 3.0;

                if(radar_gokus[gid])
                    current.score_in  += g_score * 2.0;
                else
                    current.score_out += g_score;
            }

            u += d;
        }

        if(current.score_in <= 0.0 and current.score_out <= 0.0)
            return;

        if(current.score_in > current.score_out)
        {
            if(current.score_in > kame.score_in and current.score_in > kame.score_out)
                kame = current;
        }
        else if(current.score_out > kame.score_in and current.score_out > kame.score_out)
                kame = current;
    }

    Dir get_kame_direction(Pos u, const vector<bool> &radar_gokus)
    {
        KameDir kame;
        double potential_balls = double(nb_rounds() - round()) / 30.0;
        
        kame.score_out = -potential_balls;
        kame.score_in = -potential_balls + double(gme.balls);
        kame.score_in += (double(gme.strength) / double(max_strength()));

        for(int i = Top; i <= Right; ++i)
            kame_dir_update(static_cast<Dir>(i), radar_gokus, kame);

        return kame.dir;
    }

    bool objectives_detected(Dir &kame)
    {
        if(prob_kame() < 0.75)
            return false;

        vector<bool> gokus;
                
        radar(path.end, 8, gokus);
        radar(gme.pos, 3, gokus);

        kame = get_kame_direction(gme.pos, gokus);

        return (kame != None);
    }

    void recover_energy()
    {
        Path bean(gme.pos, Bean);

        if(has_ball(gme.type) and bean.size >= path.size)
            return;

        set_path(bean);
    }

    double get_strength_prop()
    {
        return double(gme.strength) / double(max_strength());
    }

    bool is_kinton_awesome_for(Path &kin, Path &obj)
    {
        if(kin.empty())
            return false;

        if(obj.empty())
            return true;

        Path kin_obj(kin.end, obj.type);
        double heuristic = double(kin.size) + (double(kin_obj.size) / 2.0);

        return (heuristic <= 2.0 * double(obj.size));
    }

    bool is_better(Path &alt, Path &obj, double m)
    {
        if(alt.empty())
            return false;

        if(obj.empty())
            return true;

        Path alt_obj(alt.end, obj.type);

        double heuristic = double(alt.size + alt_obj.size);

        return (heuristic * m <= double(obj.size));
    }

    bool is_better_on_kinton(Path &alt, Path &obj, double m)
    {
        if(alt.empty())
            return false;

        if(obj.empty())
            return true;

        Path alt_obj(alt.end, obj.type);

        double heuristic = double(alt.size + alt_obj.size) / 2.0;

        return (heuristic <= (1.5 - m) * 1.5 * double(obj.size) / 2.0);
    }

    bool kinton_vanishes_soon()
    {
        return (double(gme.kinton) / double(kinton_life_time()) <= 0.2);
    }

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

    /**
     * @todo Depending from the number of gokus near the objective, add an heuristic to
     * the path and choose consequently.
     */
    void collect_balls_normal()
    {
        Path ball(gme.pos, Ball);
        Path kinton(gme.pos, Kinton);
        Path bean(gme.pos, Bean);

        if(is_better(bean, ball, get_strength_prop()))
        {
            if(is_kinton_awesome_for(kinton, bean))
                set_path(kinton);
            else
                set_path(bean);
        }

        // Always look for kintons! (OP)
        if(is_kinton_awesome_for(kinton, ball))
            set_path(kinton);

        else if(not ball.empty())
            set_path(ball);

        else if(not bean.empty())
            set_path(bean);
    }

    void collect_balls_with_ball()
    {
        Path capsule(gme.pos, Capsule);
        Path kinton(gme.pos, Kinton);
        Path bean(gme.pos, Bean);

        if(is_better(bean, capsule, get_strength_prop()))
        {
            if(is_kinton_awesome_for(kinton, bean))
                set_path(kinton);
            else
                set_path(bean);
        }

        // Always look for kintons! (OP)
        else if(is_kinton_awesome_for(kinton, capsule))
            set_path(kinton);

        else if(not capsule.empty())
            set_path(capsule);
    }

    void collect_balls_on_kinton()
    {
        if(kinton_vanishes_soon())
        {
            collect_balls_normal();
            return;
        }

        Path ball(gme.pos, Ball);
        Path bean(gme.pos, Bean);

        double energy = double(gme.strength) / (double(max_strength())*1.3);

        if(kamehame_penalty() > gme.strength)
            energy /= 2;

        if(is_better_on_kinton(bean, ball, energy))
            set_path(bean);

        else if(not ball.empty())
            set_path(ball);
    }

    void collect_balls_on_kinton_with_ball()
    {
        if(kinton_vanishes_soon())
        {
            collect_balls_with_ball();
            return;
        }

        Path capsule(gme.pos, Capsule);
        Path bean(gme.pos, Bean);

        double energy = double(gme.strength) / double(max_strength());

        if(is_better_on_kinton(bean, capsule, energy))
            set_path(bean);
        
        else if(not capsule.empty())
            set_path(capsule);
    }

    void wander()
    {
        Dir a[] = {Left, Right, Bottom, Top};
        Pos u;

        do {
            Dir dir = a[randomize() % 4];
            u = gme.pos + dir;
        } while(not pos_ok(u) or cell(u).type == Rock);

        path.next = u;
        path.size = 1;
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
            inst = this;

        gme = goku(me()); // Player goku shortcut
        path = Path(); // Clear the current path

        if(has_turn())
        {
            collect_balls();

            Dir kame;

            if(path.empty())
                wander();

            else if(objectives_detected(kame))
                throw_kamehame(kame);

            else
                move(get_direction_to(path.next));
        }
    }
    
};

PLAYER_NAME* PLAYER_NAME::inst = NULL;


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
