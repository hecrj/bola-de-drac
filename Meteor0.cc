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
#define PLAYER_NAME Meteor0

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
    typedef queue<Pos> IndexedQueue;

    struct Path {
        Pos next;
        Pos obj;
        int size;

        /**
         * Default constructor (0,0).
         */
        inline Path ()
        :   next(-1, -1),
            obj(-1, -1),
            size(0)
        {   }

        /**
         * Given constructor.
         */
        inline Path (Pos pos, Pos obj, int size)
        :   next(pos),
            obj(obj),
            size(size)
        {   }

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

    Path generate_path(Pos u)
    {
        Path res;
        res.size = 0;
        res.obj = u;

        Pos n = u;

        do {
            res.size++;
            u = n;
            n = map[u.i][u.j];
        } while(map[n.i][n.j].i != -1);
        
        res.next = u;

        return res;
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

    bool search(CType item, Path &path)
    {
        // Minimum distances to each Cell
        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));

        // Previous position to make paths
        map = NodeTable(rows(), vector<Pos>(cols()));

        // Pending positions to visit
        pending = IndexedQueue();

        // Player is here!
        Pos pos = gme.pos;

        costs[pos.i][pos.j] = 0;
        map[pos.i][pos.j] = Pos(-1, -1);

        pending.push(pos);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(cell(u).type == item)
            {
                path = generate_path(u);
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

    bool radar(const Pos &u, int r, queue<GInfo> &gokus, Dir &kame)
    {
        // Minimum distances to each cell
        costs = CostTable(rows(), vector<int>(cols(), numeric_limits<int>::max()));

        // Pending positions to visit
        pending = IndexedQueue();

        costs[u.i][u.j] = 0;

        pending.push(u);

        // Reset counter
        kame = None;
        vector<int> aligned(5, 0);

        while(not pending.empty())
        {
            Pos u = pending.front();
            pending.pop();

            if(costs[u.i][u.j] > r)
                break;

            int gid = get_goku_id_in(u);

            if(gid >= 0)
            {
                GInfo info;
                info.id = gid;
                info.distance = costs[u.i][u.j];

                Dir aligned_dir = get_aligned_dir(gid);

                if(aligned_dir != None)
                    aligned[aligned_dir]++;

                gokus.push(info);
            }

            for(int i = Top; i <= Right; ++i)
                push_adjacent(u, static_cast<Dir>(i));
        }

        kame = static_cast<Dir>(get_max_index(aligned));

        return gokus.size() != 0;
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

    bool try_get_kinton(const Path &obj)
    {
        Path kin;
        search(Kinton, kin);

        if(3 * kin.size <= 2*obj.size)
        {
            set_path(kin);
            return true;
        }

        set_path(obj);

        return false;
    }

    double prob_kame()
    {
        return
            double(1 + gme.strength - kamehame_penalty()) /
            double(1 + max_strength() - kamehame_penalty());
    }

    bool objectives_detected(Dir &kame)
    {
        if(prob_kame() < 0.8)
            return false;

        queue<GInfo> gokus;
                
        if(! radar(path.obj, 8, gokus, kame))
            return false;

        return (kame != None);
    }

    void recover_energy()
    {
        Path bean_path;

        search(Bean, bean_path);

        if(has_ball(gme.type) and bean_path.size >= path.size)
            return;

        set_path(bean_path);
    }

    void collect_balls()
    {
        Path obj;
        Path alt;

        if(has_ball(gme.type))
            search(Capsule, obj);
        else
            search(Ball, obj);

        if(not has_kinton(gme.type))
            try_get_kinton(obj);

        else if(is_strength_lower(0.3))
        {
            search(Bean, alt);

            if(alt.size < obj.size)
                set_path(alt);
            else
                set_path(obj);
        }
        else
            set_path(obj);

        if(path.empty())
        {
            search(Kinton, alt);
            set_path(alt);
        }
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

    void analyze_environment()
    {
        
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
        gme = goku(me()); // Player goku shortcut
        path = Path(); // Clear the current path

        // Analyze the current map for the current state
        analyze_environment();

        if(has_turn())
        {
            collect_balls();

            if(is_strength_lower(0.1))
                recover_energy();

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


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
