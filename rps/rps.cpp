#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/currency.hpp>
#include <eosiolib/time.hpp>
#include <string>
#include "gamerules.cpp"

using namespace eosio;
const checksum256 EMPTY_CHECKSUM = {0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t MOVES_IN_FIGHT = 3;
const uint8_t NEED_TO_WIN = 2;
class rps : public eosio::contract
{
public:
  using contract::contract;
  rps(account_name self) : contract(self),
                           games_table(self, self),
                           accounts_table(self, self)
  {
  }
  const static uint32_t AFK_TIME = 2 * 60; // 2 minutes
  static void assert_bet(asset bet)
  {
    eosio_assert(bet.symbol == S(4, EOS), "only accepts EOS for deposits");
    eosio_assert(bet.is_valid(), "Invalid token transfer");
    eosio_assert(bet.amount > 0, "Quantity must be positive");
  }

  static bool expired(eosio::time_point_sec seen)
  {
    return eosio::time_point_sec(now() - AFK_TIME) > seen;
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
    std::string fight1;
    std::string fight2;
    eosio::time_point_sec lastseen1;
    eosio::time_point_sec lastseen2;
    uint8_t round;
    uint64_t primary_key() const { return id; };

    EOSLIB_SERIALIZE(game, (id)(player1)(player2)(bet)(commitment1)(commitment2)(fight1)(fight2)(lastseen1)(lastseen2)(round))
  };

  typedef eosio::multi_index<N(games), game> games_index;

  // @abi table
  struct account
  {
    account_name player;
    uint32_t games;
    uint32_t wins;
    uint8_t winstreak;
    uint32_t score;
    account_name primary_key() const { return player; }
    uint64_t by_score() const { return 0xffffffffffffffff - score; }
    uint64_t by_games() const { return 0xffffffffffffffff - games; }

    EOSLIB_SERIALIZE(account, (player)(games)(wins)(winstreak)(score))
  };

  typedef eosio::multi_index<N(accounts), account,
                             indexed_by<N(score), const_mem_fun<account, uint64_t, &account::by_score>>,
                             indexed_by<N(games), const_mem_fun<account, uint64_t, &account::by_games>>>
      accounts_index;

  games_index games_table;
  accounts_index accounts_table;
  void handleWinner(account_name winner, const game &game_row, account_name payer)
  {
    auto prize = game_row.bet;
    prize.amount *= 2;
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        currency::transfer{_self, winner, prize, std::string("win:") + std::to_string(game_row.id)})
        .send();

    auto account_row = accounts_table.find(winner);
    if (account_row == accounts_table.end())
    {
      accounts_table.emplace(payer, [&](account &a) {
        a.player = winner;
        a.games = 1;
        a.wins = 1;
        a.winstreak = 1;
        a.score += a.winstreak;
      });
    }
    else
    {
      accounts_table.modify(account_row, payer, [&](account &a) {
        a.games++;
        a.wins++;
        a.winstreak++;
        a.score += a.winstreak;
      });
    }
  }
  void handleLooser(account_name looser, account_name payer)
  {
    auto account_row = accounts_table.find(looser);
    if (account_row == accounts_table.end())
    {
      accounts_table.emplace(payer, [&](account &a) {
        a.player = looser;
        a.games = 1;
        a.wins = 0;
        a.winstreak = 0;
        a.score = 0;
      });
    }
    else
    {
      accounts_table.modify(account_row, payer, [&](account &a) {
        a.games++;
        a.winstreak = 0;
      });
    }
  }
  void playGame(uint64_t gameid, account_name payer)
  {
    // play game
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    if ((game_row->fight1 == "") || (game_row->fight2 == ""))
    {
      return;
    }
    auto result = gamerules::calcWinner(game_row->fight1, game_row->fight2);

    if (result == 0)
    { // если ничья - обнуляем ходы и пусть игроки ходят заново
      return games_table.modify(game_row, payer, [&](auto &g) {
        g.fight1 = "";
        g.fight2 = "";
        g.commitment1 = EMPTY_CHECKSUM;
        g.commitment2 = EMPTY_CHECKSUM;
        g.lastseen1 = g.lastseen2 = eosio::time_point_sec(now());
        g.round++;
      });
    }
    auto winner = (result == 1) ? game_row->player1 : game_row->player2;
    auto looser = (result == 2) ? game_row->player1 : game_row->player2;
    handleWinner(winner, *game_row, payer);
    handleLooser(looser, payer);
    games_table.erase(game_row); // удаляем строчку с игрой
  }
  // @abi action
  void claimexpired(const account_name player, uint64_t gameid)
  {
    require_auth(player);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    eosio_assert(game_row->player2 != N(), "player2 should connect to game");

    if (player == game_row->player1)
    {
      eosio_assert(expired(game_row->lastseen2), "player not afk");
      handleWinner(player, *game_row, player);
      games_table.erase(game_row); // удаляем строчку с игрой
    }
    else if (player == game_row->player2)
    {
      eosio_assert(expired(game_row->lastseen1), "player not afk");
      handleWinner(player, *game_row, player);
      games_table.erase(game_row); // удаляем строчку с игрой
    }
    else
    {
      eosio_assert(false, "wrong player");
    }
  }
  // @abi action
  void cancelgame(const account_name player, uint64_t gameid)
  {
    require_auth(player);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    eosio_assert(game_row->player2 == N(), "cant cancel game with second player");
    eosio_assert(game_row->player1 == player, "only player1 can cancel game");
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        currency::transfer{_self, player, game_row->bet, "return bet"})
        .send();
    games_table.erase(game_row); // удаляем строчку с игрой
  }
  // @abi action
  void revealmove(const account_name player, uint64_t gameid, std::string fight, std::string secret)
  {
    require_auth(player);
    gamerules::assert_fight(fight);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    eosio_assert(
        !((game_row->commitment1 == EMPTY_CHECKSUM) || (game_row->commitment2 == EMPTY_CHECKSUM)),
        "both players shoud commit their hashed moves");
    // немного C++ магии
    const std::string checkstring = fight + secret;
    char data[checkstring.length()];
    strcpy(data, checkstring.c_str());
    if (player == game_row->player1)
    {
      eosio_assert(game_row->fight1 == "", "already revealed");
      assert_sha256(data, sizeof(data), (const checksum256 *)&game_row->commitment1);
      games_table.modify(game_row, player, [&](auto &g) {
        g.fight1 = fight;
        g.lastseen1 = eosio::time_point_sec(now());
      });
    }
    else if (player == game_row->player2)
    {
      eosio_assert(game_row->fight2 == "", "already revealed");
      assert_sha256(data, sizeof(data), &game_row->commitment2);
      games_table.modify(game_row, player, [&](auto &g) {
        g.fight2 = fight;
        g.lastseen2 = eosio::time_point_sec(now());
      });
    }
    else
    {
      eosio_assert(false, "wrong player");
    }

    playGame(gameid, player);
  }
  // @abi action
  void commitmove(const account_name player, uint64_t gameid, const checksum256 &commitment)
  {
    require_auth(player);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    if (player == game_row->player1)
    {
      eosio_assert(game_row->commitment1 == EMPTY_CHECKSUM, "already commited");
      games_table.modify(game_row, player, [&](auto &g) {
        g.commitment1 = commitment;
        g.lastseen1 = eosio::time_point_sec(now());
      });
    }
    else if (player == game_row->player2)
    {
      eosio_assert(game_row->commitment2 == EMPTY_CHECKSUM, "already commited");
      games_table.modify(game_row, player, [&](auto &g) {
        g.commitment2 = commitment;
        g.lastseen2 = eosio::time_point_sec(now());
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
      g.lastseen1 = g.lastseen2 = eosio::time_point_sec(0);
      g.bet = bet;
      g.round = 1;
    });
  };
  // transfer action
  void joinGame(const account_name player2, const asset bet, uint64_t gameid)
  {
    assert_bet(bet);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    eosio_assert(game_row->player1 != player2, "same players");
    eosio_assert(game_row->player2 == N(), "Game alrady have second player");
    eosio_assert(game_row->bet == bet, "bet must be same");
    games_table.modify(game_row, _self, [&](auto &g) {
      g.player2 = player2;
      g.lastseen1 = g.lastseen2 = eosio::time_point_sec(now()); // начинаем отсчет для обоих с этого момента
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
EOSIO_ABI(rps, (transfer)(commitmove)(revealmove)(cancelgame)(claimexpired))
