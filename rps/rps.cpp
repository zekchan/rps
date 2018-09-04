#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <string>

using namespace eosio;

class rps : public eosio::contract
{
public:
  using contract::contract;
  rps(account_name self) : contract(self) {}
  static void check_move_value(char value)
  {
    eosio_assert(
        (value == 'R') ||
            (value == 'P') ||
            (value == 'S'),
        "move must be less then 'R', 'P' or 'S'");
  }
  static bool first_wins(char move1, char move2)
  {
    /*
       R - rock
       P - papper
       S - scissors
      */
    if (move1 == 'R')
    {
      return move2 == 'S';
    }
    if (move1 == 'P')
    {
      return move2 == 'R';
    }
    if (move1 == 'S')
    {
      return move2 == 'P';
    }
    return true;
  };
  // @abi table
  struct move
  {
    uint64_t id;
    checksum256 commitment;
    std::string secret;
    char value;
    uint64_t primary_key() const { return id; };
    void check_value(char check_value, std::string check_secret) const
    {
      check_secret.push_back(check_value);
      assert_sha256(check_secret.c_str(), check_secret.length(), &commitment);
    }
    EOSLIB_SERIALIZE(move, (id)(commitment)(secret)(value))
  };

  typedef eosio::multi_index<N(moves), move> move_table;
  // @abi table
  struct game
  {
    uint64_t id;
    account_name player1;
    account_name player2;
    uint64_t move1;
    uint64_t move2;
    uint64_t primary_key() const { return id; };

    EOSLIB_SERIALIZE(game, (player1)(player2)(move1)(move2))
  };

  typedef eosio::multi_index<N(games), game> game_table;

  // @abi action
  void startgame(account_name player1)
  {
    require_auth(player1);
    auto games = game_table(_self, _self);
    games.emplace(player1, [&](game &g) {
      g.id = games.available_primary_key();
      g.player1 = player1;
      g.player2 = 0;
      g.move1 = 0;
      g.move2 = 0;
    });
  };
  // @abi action
  void joingame(account_name player2, uint64_t game_id)
  {
    require_auth(player2);
    auto games = game_table(_self, _self);
    auto g = games.get(game_id);
    eosio_assert(g.player2 == 0, "Game already have second player");
    games.modify(games.iterator_to(g), player2, [&](game &g) {
      g.player2 = player2;
    });
  };
  // @abi action
  void offermove(account_name player, uint64_t game_id, checksum256 commitment)
  {
    require_auth(player);
    auto games = game_table(_self, _self);
    auto moves = move_table(_self, _self);
    auto g = games.get(game_id);
    eosio_assert((g.player1 == player) || (g.player2 == player), "Player dont play this game");
    eosio_assert(((g.player1 == player) ? g.move1 : g.move2) == 0, "Move already exists");
    auto move_id = moves.available_primary_key();
    moves.emplace(player, [&](move &m) {
      m.id = move_id;
      m.commitment = commitment;
    });
    games.modify(games.iterator_to(g), player, [&](game &g) {
      if (g.player1 == player)
      {
        g.move1 = move_id;
      }
      else
      {
        g.move2 = move_id;
      }
    });
  }
  // @abi action
  void revealmove(account_name player, uint64_t game_id, std::string secret, char real_value)
  {
    require_auth(player);
    auto games = game_table(_self, _self);
    auto moves = move_table(_self, _self);
    auto g = games.get(game_id);
    eosio_assert((g.player1 == player) || (g.player2 == player), "Player dont play this game");
    eosio_assert(g.move1 != 0, "Moves not complete");
    eosio_assert(g.move2 != 0, "Moves not complete");
    auto move_id = (g.player1 == player) ? g.move1 : g.move2;
    auto m = moves.get(move_id);
    eosio_assert(m.value != 0, "Move already revealed");
    m.check_value(real_value, secret);
    moves.modify(moves.iterator_to(m), player, [&](move &m) {
      m.secret = secret;
      m.value = real_value;
    });
  }
};

EOSIO_ABI(rps, (startgame)(joingame)(offermove)(revealmove))
