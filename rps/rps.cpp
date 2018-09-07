#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/currency.hpp>
#include <string>

using namespace eosio;
const checksum256 EMPTY_CHECKSUM = {0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0};
class rps : public eosio::contract
{
public:
  using contract::contract;
  rps(account_name self) : contract(self), games_table(self, self) {}
  static void assert_bet(asset bet)
  {
    eosio_assert(bet.symbol == S(4, EOS), "only accepts EOS for deposits");
    eosio_assert(bet.is_valid(), "Invalid token transfer");
    eosio_assert(bet.amount > 0, "Quantity must be positive");
  }
  static void assert_move(uint8_t move)
  {
    eosio_assert((move >= 0) && (move <= 5), "incorect move value");
  }
  static uint8_t calcWinner(uint8_t move1, uint8_t move2)
  {
    /*
      1 - ROCK
      2 - PAPER
      3 - SCISORS
      4 - LIZARD
      5 - SPOCK
    */
    if (move1 == move2)
    {
      return 0;
    }
    if (move1 == 1)
    {
      return ((move2 == 3) || (move2 == 4)) ? 1 : 2;
    }
    if (move1 == 2)
    {
      return ((move2 == 1) || (move2 == 5)) ? 1 : 2;
    }
    if (move1 == 3)
    {
      return ((move2 == 2) || (move2 == 4)) ? 1 : 2;
    }
    if (move1 == 4)
    {
      return ((move2 == 2) || (move2 == 5)) ? 1 : 2;
    }
    if (move1 == 5)
    {
      return ((move2 == 1) || (move2 == 3)) ? 1 : 2;
    }
    return 0;
  }
  // @abi table
  struct game
  {
    uint64_t id;
    account_name player1;
    account_name player2;
    asset bet;
    checksum256 commitment1;
    checksum256 commitment2;
    uint8_t move1;
    uint8_t move2;
    uint8_t round;
    uint64_t primary_key() const { return id; };

    EOSLIB_SERIALIZE(game, (id)(player1)(player2)(bet)(commitment1)(commitment2)(move1)(move2)(round))
  };

  typedef eosio::multi_index<N(games), game> games_index;

  games_index games_table;

  void playGame(uint64_t gameid)
  {
    // play game
    auto &game_row = games_table.get(gameid);
    if (!(game_row.move1 && game_row.move2))
    {
      return;
    }
    auto result = calcWinner(game_row.move1, game_row.move2);

    if (result == 0)
    { // если ничья - обнуляем ходы и пусть игроки ходят заново
      return games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
        g.move1 = 0;
        g.move2 = 0;
        g.commitment1 = EMPTY_CHECKSUM;
        g.commitment2 = EMPTY_CHECKSUM;
        g.round++;
      });
    }
    auto winner = (result == 1) ? game_row.player1 : game_row.player2;
    auto prize = game_row.bet;
    prize.amount *= 2;
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        currency::transfer{_self, winner, prize, std::string("win:") + std::to_string(game_row.id)})
        .send();
    games_table.erase(games_table.iterator_to(game_row)); // удаляем строчку с игрой
  }
  // @abi action
  void cancelgame(const account_name player, uint64_t gameid)
  {
    require_auth(player);
    auto &game_row = games_table.get(gameid);
    eosio_assert(game_row.player2 == N(), "cant cancel game with second player");
    eosio_assert(game_row.player1 == player, "only player1 can cancel game");
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        currency::transfer{_self, player, game_row.bet, "return bet"})
        .send();
    games_table.erase(games_table.iterator_to(game_row)); // удаляем строчку с игрой    
  }
  // @abi action
  void revealmove(const account_name player, uint64_t gameid, uint8_t move, std::string secret)
  {
    require_auth(player);
    assert_move(move);
    auto &game_row = games_table.get(gameid);
    eosio_assert(
        !((game_row.commitment1 == EMPTY_CHECKSUM) || (game_row.commitment2 == EMPTY_CHECKSUM)),
        "both players shoud commit their hashed moves");
    // немного C++ магии
    const std::string checkstring = std::to_string(move) + secret;
    char data[checkstring.length()];
    strcpy(data, checkstring.c_str());
    if (player == game_row.player1)
    {
      eosio_assert(game_row.move1 == 0, "already revealed");
      assert_sha256(data, sizeof(data), (const checksum256 *)&game_row.commitment1);
      games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
        g.move1 = move;
      });
    }
    else if (player == game_row.player2)
    {
      eosio_assert(game_row.move2 == 0, "already revealed");
      assert_sha256(data, sizeof(data), &game_row.commitment2);
      games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
        g.move2 = move;
      });
    }
    else
    {
      eosio_assert(false, "wrong player");
    }

    playGame(gameid);
  }
  // @abi action
  void commitmove(const account_name player, uint64_t gameid, const checksum256 &commitment)
  {
    require_auth(player);
    auto &game_row = games_table.get(gameid);
    if (player == game_row.player1)
    {
      eosio_assert(game_row.commitment1 == EMPTY_CHECKSUM, "already commited");
      games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
        g.commitment1 = commitment;
      });
    }
    else if (player == game_row.player2)
    {
      eosio_assert(game_row.commitment2 == EMPTY_CHECKSUM, "already commited");
      games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
        g.commitment2 = commitment;
      });
    }
    else
    {
      eosio_assert(false, "wrong player");
    }
  }
  // transfer action
  void startGame(const account_name player1, const asset bet)
  {
    games_table.emplace(_self, [&](game &g) {
      g.id = games_table.available_primary_key();
      g.player1 = player1;
      g.commitment1 = EMPTY_CHECKSUM;
      g.commitment2 = EMPTY_CHECKSUM;
      g.bet = bet;
      g.round = 1;
    });
  };
  // transfer action
  void joinGame(const account_name player2, const asset bet, uint64_t gameid)
  {
    assert_bet(bet);
    auto &game_row = games_table.get(gameid);
    eosio_assert(game_row.player1 != player2, "same players");
    eosio_assert(game_row.player2 == N(), "Game alrady have second player");
    eosio_assert(game_row.bet == bet, "bet must be same");
    games_table.modify(games_table.iterator_to(game_row), _self, [&](auto &g) {
      g.player2 = player2;
    });
  }
  // eosio.token transfer handler
  void transfer(const account_name from, const account_name to, const asset &quantity, const std::string memo)
  {
    require_auth(from);
    if (from == _self)
    {
      // вывод токенов - просто разрещаем (В будующем надо добавить еще логики)
      return;
    }
    assert_bet(quantity);
    size_t del = memo.find(':');
    auto action = memo.substr(0, del);
    if (action == "create")
    {
      startGame(from, quantity);
    }
    else if (action == "join")
    {
      uint64_t gameId = std::stoull(memo.substr(del + 1));
      joinGame(from, quantity, gameId);
    }
    else
    {
      eosio_assert(false, "wrong cmd");
    }
  }
};

#undef EOSIO_ABI

#define EOSIO_ABI(TYPE, MEMBERS)                                                                                         \
  extern "C"                                                                                                             \
  {                                                                                                                      \
    void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                        \
    {                                                                                                                    \
      if (action == N(onerror))                                                                                          \
      {                                                                                                                  \
        /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
        eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");             \
      }                                                                                                                  \
      auto self = receiver;                                                                                              \
      if (code == self || code == N(eosio.token) || action == N(onerror))                                                \
      {                                                                                                                  \
        TYPE thiscontract(self);                                                                                         \
        switch (action)                                                                                                  \
        {                                                                                                                \
          EOSIO_API(TYPE, MEMBERS)                                                                                       \
        }                                                                                                                \
        /* does not allow destructor of thiscontract to run: eosio_exit(0); */                                           \
      }                                                                                                                  \
    }                                                                                                                    \
  }
EOSIO_ABI(rps, (transfer)(commitmove)(revealmove)(cancelgame))
