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
  void start_game(account_name player1)
  {
    require_auth(player1);
    auto games = game_table(_self, _self);
    games.emplace(player1, [&](game &g) {
      g.id = games.available_primary_key();
      g.player1 = player1;
      g.player2 = N();
    });
  };
  // @abi action
  void join_game(account_name player2, uint64_t game_id)
  {
    require_auth(player2);
    auto games = game_table(_self, _self);
    auto g = games.get(game_id);
    eosio_assert(g.player2 == N(), "Game already have second player");
    games.modify(games.iterator_to(g), player2, [&](game &g) {
      g.player2 = player2;
    });
  };
};

EOSIO_ABI(rps, (start_game)(join_game))
