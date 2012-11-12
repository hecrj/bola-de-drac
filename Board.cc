#include <algorithm>

#include "Board.hh"
#include "Action.hh"

#include <cctype>

using namespace std;



Board::Board (istream& is) {
  string s;

  string version;
  is >> s >> version;
  assert(s == "boladedrac");
  assert(version == "v1");

  is >> s >> nb_players_;
  assert(s == "nb_players");
  assert(nb_players_ >= 1);
  assert(nb_players_ <= 4);

  is >> s >> nb_rounds_;
  assert(s == "nb_rounds");
  assert(nb_rounds_ >= 1);
  assert(nb_rounds_ % 2 == 1); // To simplify visualization stuff.

  is >> s >> nb_capsules_;
  assert(s == "nb_capsules");
  assert(nb_capsules_ >= 1);

  is >> s >> nb_balls_;
  assert(s == "nb_balls");
  assert(nb_balls_ >= 1);

  is >> s >> nb_beans_;
  assert(s == "nb_beans");
  assert(nb_beans_ >= 0);

  is >> s >> nb_kintons_;
  assert(s == "nb_kintons");
  assert(nb_kintons_ >= 0);

  is >> s >> goku_regen_time_;
  assert(s == "goku_regen_time");
  assert(goku_regen_time_ >= 1);

  is >> s >> bean_regen_time_;
  assert(s == "bean_regen_time");
  assert(bean_regen_time_ >= 1);

  is >> s >> kinton_regen_time_;
  assert(s == "kinton_regen_time");
  assert(kinton_regen_time_ >= 1);

  is >> s >> kinton_life_time_;
  assert(s == "kinton_life_time");
  assert(kinton_life_time_ >= 1);

  is >> s >> max_strength_;
  assert(s == "max_strength");
  assert(max_strength_ >= 1);

  is >> s >> res_strength_;
  assert(s == "res_strength");
  assert(res_strength_ >= 1);
  assert(res_strength_ <= max_strength_);

  is >> s >> moving_penalty_;
  assert(s == "moving_penalty");
  assert(moving_penalty_ >= 0);

  is >> s >> kamehame_penalty_;
  assert(s == "kamehame_penalty");
  assert(kamehame_penalty_ >= 0);

  is >> s >> combat_penalty_;
  assert(s == "combat_penalty");
  assert(combat_penalty_ >= 0);

  is >> s >> rows_;
  assert(s == "rows");
  assert(rows_ >= 1);

  is >> s >> cols_;
  assert(s == "cols");
  assert(cols_ >= 1);

  names_ = vector<string>(nb_players_);
  is >> s;
  assert(s == "names");
  for (int pl = 0; pl < nb_players_; ++pl)
    is >> names_[pl];


  is >> s >> round_;
  assert(s == "round");
  assert(round_ >= 0);
  assert(round_ < nb_rounds_);

  cells_ = vector< vector<Cell> >(rows_, vector<Cell>(cols_));

  vector< vector<bool> > empty  = vector< vector<bool> >(rows_, vector<bool>(cols_, true));

  gokus_   = vector<Goku>        (nb_players_);
  kintons_ = vector<Kinton_Cloud>(nb_kintons_);
  beans_   = vector<Magic_Bean>  (nb_beans_);

  // The map shows empty cells, rocks, capsules and balls.
  // Kintons and beans are not drawn on the map but listed later.
  int n_capsules = 0;
  int n_balls    = 0;
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      cells_[i][j].id = -1;
      cells_[i][j].pos = Pos(i, j);
      char c;
      is >> c;
      c = toupper(c);
      // cerr << c << endl;
      switch (c) {
      case '.': cells_[i][j].type = Empty;                  break;
      case 'X': cells_[i][j].type = Rock;                   break;
      case 'C': cells_[i][j].type = Capsule;  ++n_capsules; break;
      case 'B': cells_[i][j].type = Ball;     ++n_balls;    break;
      default:  _unreachable();
      }
      if (cells_[i][j].type != Empty) empty[i][j] = false;
      assert((i == 0)       implies (cells_[i][j].type == Rock));
      assert((i == rows_-1) implies (cells_[i][j].type == Rock));
      assert((j == 0)       implies (cells_[i][j].type == Rock));
      assert((j == cols_-1) implies (cells_[i][j].type == Rock));
    }
  }
  assert(n_capsules == nb_capsules());

  is >> s;
  assert(s == "beans");
  beans_ = vector<Magic_Bean>(nb_beans_);
  for (int k = 0; k < nb_beans_; ++k) {
    int i, j, time;
    char c;
    is >> i >> j >> c >> time;
    assert(pos_ok(i, j));
    assert(c == 'y' or c == 'n');
    assert(time >= 0);
    beans_[k].pos.i   = i;
    beans_[k].pos.j   = j;
    beans_[k].present = (c == 'y');
    beans_[k].time    = time;
    assert(empty[i][j]);
    empty[i][j] = false;
    if (beans_[k].present) cells_[i][j].type = Bean;
  }

  is >> s;
  assert(s == "kintons");
  kintons_ = vector<Kinton_Cloud>(nb_kintons_);
  for (int k = 0; k < nb_kintons_; ++k) {
    int i, j, time;
    char c;
    is >> i >> j >> c >> time;
    assert(pos_ok(i, j));
    assert(time >= 0);
    assert(c == 'y' or c == 'n');
    kintons_[k].pos.i   = i;
    kintons_[k].pos.j   = j;
    kintons_[k].present = (c == 'y');
    kintons_[k].time    = time;
    assert(empty[i][j]);
    empty[i][j] = false;
    if (kintons_[k].present) cells_[i][j].type = Kinton;
  }

  int n_gokus_with_ball = 0;
  status_ = vector<double>(nb_players_);
  for (int id = 0; id < nb_players_; ++id) {
    int i, j, time, balls, strength, kinton;
    double status;
    char state;
    is >> s >> i >> j >> time >> state >> balls >> strength >> kinton >> status;
    assert(pos_ok(i, j));
    assert(time >= 0);
    assert(state == 'a' or state == 'd');
    assert(balls >= 0);
    assert(0 <= strength and strength <= max_strength_);
    assert(status == -1 or (0 <= status and status <= 1));

    GType type;
    if      (s == "normal")              type = Normal;
    else if (s == "on_kinton")           type = On_Kinton;
    else if (s == "with_ball")           type = With_Ball;
    else if (s == "on_kinton_with_ball") type = On_Kinton_With_Ball;
    else _unreachable();
    assert((not has_kinton(type)) implies (kinton == 0));
    assert(     has_kinton(type)  implies (kinton  > 0));

    if (has_ball(type)) ++n_gokus_with_ball;

    gokus_[id].id       = id;
    gokus_[id].pos      = Pos(i, j);
    gokus_[id].type     = type;
    gokus_[id].balls    = balls;
    gokus_[id].strength = strength;
    gokus_[id].kinton   = kinton;
    gokus_[id].alive    = state == 'a';
    gokus_[id].time     = time;

    status_[id] = status;

    if (gokus_[id].alive) cells_[i][j].id = id;

    assert(not gokus_[id].alive         or 
	   cells_[i][j].type == Empty   or 
	   cells_[i][j].type == Capsule or 
	   (cells_[i][j].type == Ball and has_ball(gokus_[id].type)));
  }
  assert(n_balls + n_gokus_with_ball == nb_balls());

  nb_balls_to_be_placed_ = 0;

  assert(ok());
}



void Board::print_preamble (ostream& os) const {
  os << "boladedrac v1"                             << endl;
  os << "nb_players "        << nb_players()        << endl;
  os << "nb_rounds "         << nb_rounds()         << endl;
  os << "nb_capsules "       << nb_capsules()       << endl;
  os << "nb_balls "          << nb_balls()          << endl;
  os << "nb_beans "          << nb_beans()          << endl;
  os << "nb_kintons "        << nb_kintons()        << endl;
  os << "goku_regen_time "   << goku_regen_time()   << endl;
  os << "bean_regen_time "   << bean_regen_time()   << endl;
  os << "kinton_regen_time " << kinton_regen_time() << endl;
  os << "kinton_life_time "  << kinton_life_time()  << endl;
  os << "max_strength "      << max_strength()      << endl;
  os << "res_strength "      << res_strength()      << endl;
  os << "moving_penalty "    << moving_penalty()    << endl;
  os << "kamehame_penalty "  << kamehame_penalty()  << endl;
  os << "combat_penalty "    << combat_penalty()    << endl;
  os << "rows "              << rows()              << endl;
  os << "cols "              << cols()              << endl;
  os << "names";
  for (int player = 0; player < nb_players(); ++player) os << " " << name(player);
  os << endl << endl;
}



void Board::print (ostream& os) const {
  os << endl;
  os << "round " << round() << endl;
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      CType c = cell(i, j).type;
      switch (c) {
      case Empty  : os << '.'; break;
      case Rock   : os << 'X'; break;
      case Capsule: os << 'C'; break;
      case Ball   : os << 'B'; break;
      case Kinton : os << '.'; break;
      case Bean   : os << '.'; break;
      default     : _unreachable();
      };
    }
    os << endl;
  }
  os << "beans" << endl;
  for (int i = 0; i < nb_beans_; ++i)
    os <<   beans_[i].pos.i << " " <<   beans_[i].pos.j << " " <<   (beans_[i].present ? 'y': 'n') << " " <<   beans_[i].time << endl;
  os << "kintons" << endl;
  for (int i = 0; i < nb_kintons_; ++i)
    os << kintons_[i].pos.i << " " << kintons_[i].pos.j << " " << (kintons_[i].present ? 'y': 'n') << " " << kintons_[i].time << endl;
  for (int id = 0; id < nb_players(); ++id) {
    const Goku& g = goku(id);
    string s;
    switch (g.type) {
    case Normal             : s = "normal";              break;
    case On_Kinton          : s = "on_kinton";           break;
    case With_Ball          : s = "with_ball";           break;
    case On_Kinton_With_Ball: s = "on_kinton_with_ball"; break;
    default                 : _unreachable();
    };
    os  << s                     << " "
	<< g.pos.i               << " "
	<< g.pos.j               << " "
	<< g.time                << " "
	<< (g.alive ? 'a' : 'd') << " "
	<< g.balls               << " "
	<< g.strength            << " "
	<< g.kinton              << " "
	<< status_[id]           << " "
	<< endl;
  }
  os << endl;
}

Board Board::next (const vector<Action>& asked, vector<Action>& done) const {

  assert(ok());

  // b will be the new board we shall return
  Board b = *this;

  // increment the round
  ++b.round_;

  // reset
  b.nb_balls_to_be_placed_ = 0;

  // randomize turns
  vector<int> turns(nb_players());
  for (int player = 0; player < nb_players(); ++player) {
    turns[player] = player;
  }
  random_shuffle(turns.begin(), turns.end());

  // handle kame hames of gokus
  for (int turn = 0; turn < nb_players(); ++turn) {
    int player = turns[turn];
    assert(done[player].type == Undefined);
    assert(done[player].dir  == None     );
    AType a = asked[player].type;
    Dir   d = asked[player].dir;

    const Goku& g = gokus_[player];
    if (g.strength >= kamehame_penalty()) {
      int outcome = randomize() % (max_strength() - kamehame_penalty() + 1);
      int allowed = g.strength - kamehame_penalty();
      if (outcome <= allowed) {
	bool throw_ok = b.apply_throw(player, a, d);
	if (throw_ok) {
	  done[player].type = a;
	  done[player].dir  = d;
	  break; // only 1 kame hame per round!
	}
      }
    }
  }

  // handle movements of gokus
  for (int turn = 0; turn < nb_players(); ++turn) {
    int player = turns[turn];
    assert(done[player].type == Undefined or done[player].type == Throwing);
    assert((done[player].type == Undefined) implies (done[player].dir == None));
    assert((done[player].type == Throwing)  implies (done[player].dir != None));

    AType a = asked[player].type;
    Dir   d = asked[player].dir;
    bool move_ok = b.apply_move(player, a, d);
    if (move_ok) {
      done[player].type = a;
      done[player].dir  = d;
    }
  }

  for (int player = 0; player < nb_players(); ++player) {
    Goku& g = b.gokus_[player];

    // check for end of kinton
    if (has_kinton(g.type)) {
      assert(g.alive);
      if (--g.kinton <= 0) {
	if (g.type == On_Kinton) g.type = Normal;
	else                     g.type = With_Ball;
      }
    }

    // check for end of regeneration time of gokus
    if (not g.alive and b.status(player) >= 0) {
      if (g.time > 0) --g.time;
      if (g.time == 0 and round() % 2 == 1) {
	bool fnd = false;
	for (int reps = 0; reps < 200 and not fnd; ++reps) {
	  Pos p = Pos(randomize() % rows(), randomize() % cols());
	  fnd = (b.cell(p).type == Empty);
	  for (int ii = -2; fnd and ii <= 2; ++ii)
	    for (int jj = -2; fnd and jj <= 2; ++jj)
	      fnd = (not b.pos_ok(p.i + ii, p.j + jj)) or b.cell(p.i + ii, p.j + jj).id == -1;
	  if (fnd) {
	    b.cells_[p.i][p.j].id = player;
	    g.pos   = p;
	    g.type  = Normal;
	    g.strength = b.res_strength_;
	    g.kinton = 0;
	    g.alive = true;
	  }
	}
	if (not fnd) cerr << "info: cannot place goku on board." << endl;
      }
    }
  }

  // check for end of regeneration time of beans
  for (int i = 0; i < nb_beans(); ++i) {
    Magic_Bean& m = b.beans_[i];
    if (m.time > 0) --m.time;
    Pos p = m.pos;
    if (m.time == 0 and b.cell(p).type == Empty and b.cell(p).id == -1) {
      assert(not m.present);
      m.present = true;
      b.cells_[p.i][p.j].type = Bean;
    }
  }

  // check for end of regeneration time of kintons
  for (int i = 0; i < nb_kintons(); ++i) {
    Kinton_Cloud& k = b.kintons_[i];
    if (k.time > 0) --k.time;
    Pos p = k.pos;
    if (k.time == 0 and b.cell(p).type == Empty and b.cell(p).id == -1) {
      assert(not k.present);
      k.present = true;
      b.cells_[p.i][p.j].type = Kinton;
    }
  }

  // place new balls
  for (int i = 0; i < b.nb_balls_to_be_placed_; ++i) {
    bool placed = b.place_new_ball();
    assert(placed);
  }

  assert(b.ok());
  return b;
}


bool Board::apply_throw(int id, AType a, Dir d) {

  if (a != Throwing or d == None) return false;

  Goku& g1 = gokus_[id];
  if (not g1.alive) return false;
  if (round() % 2 == 0 and not has_kinton(g1.type)) return false;

  Pos p1 = g1.pos;
  assert(pos_ok(p1));

  if (g1.strength < kamehame_penalty()) return false;
  else {
    g1.strength -= kamehame_penalty();
    Pos p = dest(p1, d);
    assert(pos_ok(p)); // Board is surrounded by rocks.
    while (cell(p).type != Rock) {
      if (cell(p).id != -1) {
	assert(cell(p).id != id);
	Goku& g2 = gokus_[cell(p).id];
	assert(g2.alive);
	cells_[p.i][p.j].id = -1;
	if (has_ball(g2.type))
	  ++nb_balls_to_be_placed_;

	g2.type     = Normal;
	g2.strength = 0;
	g2.kinton   = 0;
	g2.alive    = false;
	g2.time     = goku_regen_time();
	cells_[p.i][p.j].id = -1;
      }
      p = dest(p, d);
    }
    return true;
  }
}


bool Board::apply_move(int id, AType a, Dir d) {

  if (a != Moving or d == None) return false;

  Goku& g1 = gokus_[id];
  if (not g1.alive) return false;
  if (round() % 2 == 0 and not has_kinton(g1.type)) return false;

  Pos p1 = g1.pos;
  assert(pos_ok(p1));

  Pos p2 = dest(p1, d);
  if (not pos_ok(p2)) return false;

  Cell& c1 = cells_[p1.i][p1.j];
  assert(c1.id == id);

  Cell& c2 = cells_[p2.i][p2.j];

  if (c2.type == Rock) return false;

  if (c2.id != -1) {

    if (not has_kinton(g1.type) and g1.strength < moving_penalty() and round() % 4 == 3) return false;

    assert(c2.id != id);
    Goku& g2 = gokus_[c2.id];
    assert(g2.alive);
    assert(c2.type == Empty or c2.type == Capsule or (c2.type == Ball and has_ball(g2.type)));
    int outcome = randomize() % (2 + g1.strength + g2.strength);
    c1.id = -1;
    if (outcome <= g1.strength) {
      c2.id = id;
      g1.pos = p2;

      g1.strength -= combat_penalty();
      if (not has_kinton(g1.type) and round() % 4 == 3) 
	g1.strength -= moving_penalty();
      g1.strength = max(0, g1.strength);

      if (has_kinton(g2.type)) {
	g1.type = jump_on_kinton(g1.type);
	g1.kinton += g2.kinton;
      }
      if (has_ball(g2.type)) {
	assert(c2.type != Capsule);
	if (has_ball(g1.type))
	  ++nb_balls_to_be_placed_;
	else if (c2.type == Ball) {
	  g1.type = take_ball(g1.type);
	  c2.type = Empty;
	  ++nb_balls_to_be_placed_;
	}
	else g1.type = take_ball(g1.type);
      }
      if (has_ball(g1.type) and c2.type == Capsule) {
	++g1.balls;
	if   (g1.type == With_Ball) g1.type = Normal;
	else                        g1.type = On_Kinton;
	++nb_balls_to_be_placed_;
      }
      g2.type     = Normal;
      g2.strength = 0;
      g2.kinton   = 0;
      g2.alive    = false;
      g2.time     = goku_regen_time();
    }
    else {

      g2.strength = max(0, g2.strength - combat_penalty());

      if (has_kinton(g1.type)) {
	g2.type = jump_on_kinton(g2.type);
	g2.kinton += g1.kinton;
      }
      if (has_ball(g1.type)) {
	if (has_ball(g2.type)) {
	  assert(c2.type != Capsule);
	  ++nb_balls_to_be_placed_;
	}
	else if (c2.type == Capsule) {
	  ++g2.balls;
	  ++nb_balls_to_be_placed_;
	}
	else g2.type = take_ball(g2.type);
      }
      g1.type     = Normal;
      g1.strength = 0;
      g1.kinton   = 0;
      g1.alive    = false;
      g1.time     = goku_regen_time();
    }
  }
  else {
    if (not has_kinton(g1.type) and round() % 4 == 3) {
      if (g1.strength < moving_penalty()) return false;
      else g1.strength -= moving_penalty();
    }
    if (c2.type == Bean) {
      g1.strength = max_strength();
      c2.type = Empty;
      int k;
      for (k = 0; k < nb_beans(); ++k)
	if (beans_[k].pos == p2) break;
      assert(k < nb_beans());
      assert(beans_[k].present);
      beans_[k].present = false;
      beans_[k].time = bean_regen_time();
    }
    else if (c2.type == Kinton) {
      g1.type = jump_on_kinton(g1.type);
      g1.kinton += kinton_life_time();
      c2.type = Empty;
      int k;
      for (k = 0; k < nb_kintons(); ++k)
	if (kintons_[k].pos == p2) break;
      assert(k < nb_kintons());
      assert(kintons_[k].present);
      kintons_[k].present = false;
      kintons_[k].time = kinton_regen_time();
    }
    else if (c2.type == Ball and not has_ball(g1.type)) {
      g1.type = take_ball(g1.type);
      c2.type = Empty;
    }
    else if (c2.type == Capsule and has_ball(g1.type)) {
      ++g1.balls;
      if   (g1.type == With_Ball) g1.type = Normal;
      else                        g1.type = On_Kinton;
      ++nb_balls_to_be_placed_;
    }
    c1.id = -1;
    c2.id = id;
    g1.pos = p2;
  }
  return true;
}


bool Board::place_new_ball(void) {
  for (int reps = 0; reps < 200; ++reps) {
    Pos p = Pos(randomize() % rows(), randomize() % cols());

    bool bok = (cell(p).type == Empty);

    for (int di = -2; bok and di <= 2; ++di)
      for (int dj = -2; bok and dj <= 2; ++dj)
	bok = (not pos_ok(p.i + di, p.j + dj)) or cell(p.i + di, p.j + dj).id == -1;

    for (int k = 0; bok and k < nb_kintons(); ++k)
      bok = (p != kintons_[k].pos);

    for (int k = 0; bok and k < nb_beans(); ++k)
      bok = (p != beans_[k].pos);

    if (bok) {
      cells_[p.i][p.j].type = Ball;
      return true;
    }
  }
  cerr << "info: cannot place ball on board." << endl;
  return false;
}


bool Board::ok(void) const {

  if (int(cells_.size()) != rows_) {
    cerr << "debug: problems with row dimension of the board." << endl;
    return false;
  }
  if (int(cells_[0].size()) != cols_) {
    cerr << "debug: problems with col dimension of the board." << endl;
    return false;
  }
  if (int(gokus_.size()) != nb_players_) {
    cerr << "debug: problems with number of players (1)." << endl;
    return false;
  }
  if (int(status_.size()) != nb_players_) {
    cerr << "debug: problems with number of players (2)." << endl;
    return false;
  }
  if (int(names_.size()) != nb_players_) {
    cerr << "debug: problems with number of players (3)." << endl;
    return false;
  }
  if (int(kintons_.size()) != nb_kintons_) {
    cerr << "debug: wrong size of vector of kintons." << endl;
    return false;
  }
  if (int(beans_.size()) != nb_beans_) {
    cerr << "debug: wrong size of vector of beans." << endl;
    return false;
  }

  set<Pos> s;
  for (int k = 0; k < nb_kintons(); ++k) s.insert(kintons_[k].pos);
  for (int k = 0; k < nb_beans()  ; ++k) s.insert(  beans_[k].pos);
  if (int(s.size()) != nb_kintons() + nb_beans()) {
    cerr << "debug: overlapping of kintons and beans." << endl;
    return false;
  }

  int n_capsules = 0;
  int n_balls_not_taken = 0;
  set<int> players_set;
  for (int i = 0; i < rows(); ++i) {
    for (int j = 0; j < cols(); ++j) {

      if (cells_[i][j].pos != Pos(i, j)) {
	cerr << "debug: problem with positions in cells." << endl;
	return false;
      }

      if (i == 0          and cells_[i][j].type != Rock) {
	cerr << "debug: borders of board should be rocks (1)." << endl;
	return false;
      }
      if (i == rows() - 1 and cells_[i][j].type != Rock) {
	cerr << "debug: borders of board should be rocks (2)." << endl;
	return false;
      }
      if (j == 0          and cells_[i][j].type != Rock) {
	cerr << "debug: borders of board should be rocks (3)." << endl;
	return false;
      }
      if (j == cols() - 1 and cells_[i][j].type != Rock) {
	cerr << "debug: borders of board should be rocks (4)." << endl;
	return false;
      }
      switch (cells_[i][j].type) {
      case Capsule: ++n_capsules;        break;
      case Ball:    ++n_balls_not_taken; break;
      default: break;
      }

      int id = cells_[i][j].id;
      if (id != -1) {
	if (id < -1 or id > nb_players_) {
	  cerr << "debug: problem with players in cells." << endl;
	  return false;
	}
	if (players_set.count(id) == 1) {
	  cerr << "debug: players are repeated on board." << endl;
	  return false;
	}
	players_set.insert(id);
	if (gokus_[id].pos != Pos(i,j)) {
	  cerr << "debug: wrong position in goku." << endl;
	  return false;
	}
	if (not gokus_[id].alive) {
	  cerr << "debug: goku should be alive." << endl;
	  return false;
	}
      }

    }
  }
  for (int id = 0; id < nb_players_; ++id) {
    if (players_set.count(id) == 0) {
      if (gokus_[id].alive) {
	cerr << "debug: goku should be dead." << endl;
	return false;
      }
    }
  }

  if (n_capsules != nb_capsules()) {
    cerr << "debug: wrong number of capsules." << endl;
    return false;
  }

  int n_gokus_with_ball = 0;
  for (int id = 0; id < nb_players_; ++id) {
    const Goku& g = gokus_[id];
    if (g.alive and has_ball(g.type)) ++n_gokus_with_ball;
  }
  if (n_gokus_with_ball + n_balls_not_taken != nb_balls()) {
    cerr << "debug: wrong number of balls." << endl;
    return false;
  }

  for (int id = 0; id < nb_players_; ++id) {
    const Goku& g = gokus_[id];
    if (g.id != id) {
      cerr << "debug: goku has wrong identifier." << endl;
      return false;
    }
    if (not pos_ok(g.pos)) {
      cerr << "debug: problem with position of goku (1)." << endl;
      return false;
    }
    if (not g.alive and g.kinton != 0) {
      cerr << "debug: invalid state for a dead goku (1)." << endl;
      return false;
    }
    if (not g.alive and g.strength != 0) {
      cerr << "debug: invalid state for a dead goku (2)." << endl;
      return false;
    }
    if (not g.alive and g.type != Normal) {
      cerr << "debug: invalid state for a dead goku (3)." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).id != id) {
      cerr << "debug: problem with position of goku (2)." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).type == Rock) {
      cerr << "debug: goku cannot be in rock." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).type == Capsule and has_ball(g.type)) {
      cerr << "debug: goku should have left the ball." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).type == Ball and not has_ball(g.type)) {
      cerr << "debug: goku should have taken the ball." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).type == Kinton) {
      cerr << "debug: goku should have jumped on kinton cloud." << endl;
      return false;
    }
    if (g.alive and cell(g.pos).type == Bean) {
      cerr << "debug: goku should have eaten magic bean." << endl;
      return false;
    }
    if (g.balls < 0) {
      cerr << "debug: number of balls cannot be negative." << endl;
      return false;
    }
    if (g.strength < 0) {
      cerr << "debug: strength cannot be negative." << endl;
      return false;
    }
    if (g.strength > max_strength()) {
      cerr << "debug: strength cannot exceed max value." << endl;
      return false;
    }
    if (g.kinton < 0) {
      cerr << "debug: kinton life time cannot be negative." << endl;
      return false;
    }
    if (has_kinton(g.type) and g.kinton == 0) {
      cerr << "debug: goku should not be on kinton." << endl;
      return false;
    }
    if (not has_kinton(g.type) and g.kinton > 0) {
      cerr << "debug: goku should be on kinton." << endl;
      return false;
    }

    if (g.time < 0) {
      cerr << "debug: regeneration time cannot be negative." << endl;
      return false;
    }
    if (g.time > goku_regen_time()) {
      cerr << "debug: regeneration time cannot exceed max value." << endl;
      return false;
    }
    if (g.alive and g.time > 0) {
      cerr << "debug: live goku cannot have non-null regeneration time." << endl;
      return false;
    }
  }

  for (int k = 0; k < nb_kintons(); ++k) {
    Pos p = kintons_[k].pos;
    if (not pos_ok(p)) {
      cerr << "debug: invalid position of a kinton cloud." << endl;
      return false;
    }
    if (kintons_[k].present and kintons_[k].time > 0) {
      cerr << "debug: regeneration time of kinton should be null." << endl;
      return false;
    }
    if (not kintons_[k].present and kintons_[k].time == 0 and cell(p).id == -1) {
      cerr << "debug: kinton should be present." << endl;
      return false;
    }
    if (cell(p).type != Empty and cell(p).type != Kinton) {
      cerr << "debug: problems with cells and kintons (1)." << endl;
      return false;
    }
    if (cell(p).type == Empty and kintons_[k].present) {
      cerr << "debug: problems with cells and kintons (2)." << endl;
      return false;
    }
    if (cell(p).type == Kinton and not kintons_[k].present) {
      cerr << "debug: problems with cells and kintons (3)." << endl;
      return false;
    }
  }

  for (int k = 0; k < nb_beans(); ++k) {
    Pos p = beans_[k].pos;
    if (not pos_ok(p)) {
      cerr << "debug: invalid position of a bean cloud." << endl;
      return false;
    }
    if (beans_[k].present and beans_[k].time > 0) {
      cerr << "debug: regeneration time of bean should be null." << endl;
      return false;
    }
    if (not beans_[k].present and beans_[k].time == 0 and cell(p).id == -1) {
      cerr << "debug: bean should be present." << endl;
      return false;
    }
    if (cell(p).type != Empty and cell(p).type != Bean) {
      cerr << "debug: problems with cells and beans (1)." << endl;
      return false;
    }
    if (cell(p).type == Empty and beans_[k].present) {
      cerr << "debug: problems with cells and beans (2)." << endl;
      return false;
    }
    if (cell(p).type == Bean and not beans_[k].present) {
      cerr << "debug: problems with cells and beans (3)." << endl;
      return false;
    }
  }

  return true;
}
