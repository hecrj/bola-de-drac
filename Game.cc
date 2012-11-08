#include "Game.hh"
#include "Registry.hh"


using namespace std;


void Game::run (vector<string> names, istream& is, ostream& os) {
  cerr << "info: loading game" << endl;
  Board b0(is);
  cerr << "info: loaded game" << endl;

  if (int(names.size()) != b0.nb_players()) {
    cerr << "fatal: wrong number of players." << endl;
    exit(EXIT_FAILURE);
  }

  vector<Player*> players;        
  for (int player = 0; player < b0.nb_players(); ++player) {
    string name = names[player];
    cerr << "info: loading player " << name << endl;
    players.push_back(Registry::new_player(name));
    b0.names_[player] = name;
  }
  cerr << "info: players loaded" << endl;

  b0.print_preamble(os);
  b0.print(os);

  for (int round = 1; round < b0.nb_rounds(); ++round) {
    cerr << "info: start round " << round << endl;
    os << "actions_asked" << endl;
    vector<Action> asked;
    for (int player = 0; player < b0.nb_players(); ++player) {
      cerr << "info:     start player " << player << endl;
      Action a;
      players[player]->reset(player, b0, a);
      players[player]->play();
      asked.push_back(*players[player]);

      os << player << " ";
      Action(*players[player]).print(os);
      cerr << "info:     end player " << player << endl;
    }
    vector<Action> done(b0.nb_players());
    cerr << "start next" << endl;
    Board b1 = b0.next(asked, done);
    cerr << "end next" << endl;

    os << endl << "actions_done" << endl;
    for (int player = 0; player < b0.nb_players(); ++player) {
      os << player << " ";
      done[player].print(os);
    }
    os << endl;

    b1.print(os);
    b0 = b1;
    cerr << "info: end round " << round << endl;
  }

  vector<int> max_players;
  int max_score = minint;

  for (int player = 0; player < b0.nb_players(); ++player) {
    cerr << "info: player " << b0.name(player) << " got score " << b0.score(player) << endl;
    if (max_score < b0.score(player)) {
      max_score = b0.score(player);
      max_players = vector<int>(1, player);
    }
    else if (max_score == b0.score(player))
      max_players.push_back(player);
  }
  cerr << "info: player(s)";
  for (int k = 0; k < int(max_players.size()); ++k)
    cerr << " " << b0.name(max_players[k]);
  cerr << " got top score" << endl;

  cerr << "info: game played" << endl;
}
