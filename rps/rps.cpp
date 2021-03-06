#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <string>
#include "gamerules.cpp"

using namespace eosio;
const checksum256 EMPTY_CHECKSUM = {0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0};
uint8_t const TAX_NUMERATOR = 2;
uint8_t const TAX_DENOMINATOR = 100;
uint64_t const AVAILEBLE_AMOUNTS[5] = {10000, 20000, 30000, 40000, 50000};
uint8_t const MAX_GAMES = 3;
account_name const EMPTY_PLAYER = N(.);
class rps : public eosio::contract
{
public:
  using contract::contract;
  rps(account_name self) : contract(self),
                           games_table(self, self),
                           accounts_table(self, self),
                           global_table(self, self)
  {
  }
  const static uint32_t AFK_TIME = 30; // 30 seconds
  static void assert_bet(asset bet)
  {
    eosio_assert(bet.symbol == S(4, EOS), "only accepts EOS for deposits");
    eosio_assert(bet.is_valid(), "Invalid token transfer");
    for (uint64_t allowed_amount : AVAILEBLE_AMOUNTS)
    {
      if (allowed_amount == bet.amount)
      {
        return;
      }
    }
    eosio_assert(false, "wrong bet");
  }

  static bool expired(eosio::time_point_sec seen)
  {
    return eosio::time_point_sec(now() - AFK_TIME) > seen;
  }
  // @abi table
  struct global
  {
    uint64_t games;
    uint64_t primary_key() const { return 0; }
    EOSLIB_SERIALIZE(global, (games))
  };
  typedef eosio::multi_index<N(globalstats), global> global_index;

  // @abi table
  struct game
  {
    uint64_t id;
    uint8_t round;
    account_name player1;
    account_name player2;
    // TODO: store only amount of bet for using less RAM
    asset bet;
    checksum256 commitment1;
    checksum256 commitment2;
    std::string fight1;
    std::string fight2;
    eosio::time_point_sec afksnapshot;
    uint64_t primary_key() const { return id; };
    account_name by_player2() const { return player2; };
    account_name by_player1() const { return player1; };

    EOSLIB_SERIALIZE(game, (id)(round)(player1)(player2)(bet)(commitment1)(commitment2)(fight1)(fight2)(afksnapshot))
  };

  typedef eosio::multi_index<N(games), game,
                             indexed_by<N(player1), const_mem_fun<game, account_name, &game::by_player1>>,
                             indexed_by<N(player2), const_mem_fun<game, account_name, &game::by_player2>>>
      games_index;
  struct gameresult
  {
    uint64_t id;
    uint8_t result;
  };
  // @abi table
  struct account
  {
    account_name player;
    uint32_t games;
    uint32_t wins;
    asset deposit;
    std::vector<gameresult> lastgames;
    uint8_t openedgames;
    account_name primary_key() const { return player; }
    uint64_t by_games() const { return 0xffffffffffffffff - games; }

    EOSLIB_SERIALIZE(account, (player)(games)(wins)(deposit)(lastgames)(openedgames))
  };

  typedef eosio::multi_index<N(accounts), account,
                             indexed_by<N(games), const_mem_fun<account, uint64_t, &account::by_games>>>
      accounts_index;

  games_index games_table;
  accounts_index accounts_table;
  global_index global_table;
  void handleWinner(account_name winner, const game &game_row)
  {
    auto quantity = game_row.bet;
    quantity.amount *= 2;
    uint64_t tax = quantity.amount * TAX_NUMERATOR / TAX_DENOMINATOR;
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        std::make_tuple(_self, winner, quantity, ""))
        .send();
    quantity.amount = tax;
    action(
        permission_level{_self, N(active)},
        N(eosio.token), N(transfer),
        std::make_tuple(_self, N(rpscommision), quantity, ""))
        .send();

    auto account_row = accounts_table.find(winner);
    accounts_table.modify(account_row, 0, [&](account &a) {
      a.games++;
      a.wins++;
      a.openedgames--;
      a.lastgames.erase(a.lastgames.begin());
      a.lastgames.push_back({game_row.id, 1});
    });
  }
  void handleLooser(account_name looser, const game &game_row)
  {
    auto account_row = accounts_table.find(looser);

    accounts_table.modify(account_row, 0, [&](account &a) {
      a.games++;
      a.openedgames--;
      a.lastgames.erase(a.lastgames.begin());
      a.lastgames.push_back(gameresult{game_row.id, 2});
    });
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
        g.afksnapshot = eosio::time_point_sec(now());
        g.round++;
      });
    }
    auto winner = (result == 1) ? game_row->player1 : game_row->player2;
    auto looser = (result == 2) ? game_row->player1 : game_row->player2;
    handleWinner(winner, *game_row);
    handleLooser(looser, *game_row);
    games_table.erase(game_row); // удаляем строчку с игрой
  }
  // @abi action
  void claimexpired(const account_name player, uint64_t gameid)
  {
    require_auth(player);
    auto game_row = games_table.find(gameid);
    eosio_assert(game_row != games_table.end(), "not found game");
    eosio_assert(game_row->player2 != EMPTY_PLAYER, "game doesnt start");
    eosio_assert(expired(game_row->afksnapshot), "not afk");

    if (player == game_row->player1)
    {
      eosio_assert(
          ((game_row->fight1 != "") && (game_row->fight2 == "")) ||
              (!(game_row->commitment1 == EMPTY_CHECKSUM) && (game_row->commitment2 == EMPTY_CHECKSUM)),
          "player2 not afk");

      handleWinner(player, *game_row);
      handleLooser(game_row->player2, *game_row);
      games_table.erase(game_row); // удаляем строчку с игрой
    }
    else if (player == game_row->player2)
    {
      eosio_assert(
          ((game_row->fight2 != "") && (game_row->fight1 == "")) ||
              (!(game_row->commitment2 == EMPTY_CHECKSUM) && (game_row->commitment1 == EMPTY_CHECKSUM)),
          "player1 not afk");

      handleWinner(player, *game_row);
      handleLooser(game_row->player1, *game_row);
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
    eosio_assert(game_row->player2 == EMPTY_PLAYER, "cant cancel game with second player");
    eosio_assert(game_row->player1 == player, "only player1 can cancel game");
    auto qnt = game_row->bet;
    qnt.amount = qnt.amount * (TAX_DENOMINATOR + TAX_NUMERATOR) / TAX_DENOMINATOR;
    action(permission_level{_self, N(active)},
           N(eosio.token), N(transfer),
           std::make_tuple(_self, player, qnt, "return bet"))
        .send();
    games_table.erase(game_row); // удаляем строчку с игрой

    auto account_row = accounts_table.find(player);
    accounts_table.modify(account_row, 0, [&](account &a) {
      a.openedgames--;
    });
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
      });
    }
    else if (player == game_row->player2)
    {
      eosio_assert(game_row->fight2 == "", "already revealed");
      assert_sha256(data, sizeof(data), &game_row->commitment2);
      games_table.modify(game_row, player, [&](auto &g) {
        g.fight2 = fight;
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
        if (!(game_row->commitment2 == EMPTY_CHECKSUM))
        {
          g.afksnapshot = eosio::time_point_sec(now());
        }
      });
    }
    else if (player == game_row->player2)
    {
      eosio_assert(game_row->commitment2 == EMPTY_CHECKSUM, "already commited");
      games_table.modify(game_row, player, [&](auto &g) {
        g.commitment2 = commitment;
        if (!(game_row->commitment1 == EMPTY_CHECKSUM))
        {
          g.afksnapshot = eosio::time_point_sec(now());
        }
      });
    }
    else
    {
      eosio_assert(false, "wrong player");
    }
  }
  // @abi action
  void startgame(const account_name player)
  {
    auto account_row = accounts_table.find(player);
    eosio_assert(account_row != accounts_table.end(), "player shoud register");
    auto bet = account_row->deposit;
    eosio_assert(bet.amount > 0, "player shoud have deposit");
    auto games_table_player2 = games_table.get_index<N(player2)>(); // Смотрим через by_player2 индекс
    /*
      Находим первую игру с пустым player2, и игру следующую за последним (может быть конец)
      Таких игр может быть максимум количество возможных ставок (Мало)
      Итерируемся между ними и ищем игру с нашей ствкой - если нашли, заходим в нее, если нет - создаем новую
    */

    accounts_table.modify(account_row, 0, [&](account &a) {
      a.deposit = asset(0);
      a.openedgames++;
    });
    auto itr = games_table_player2.lower_bound(EMPTY_PLAYER);
    auto endItr = games_table_player2.lower_bound(EMPTY_PLAYER + 1);
    while (itr != endItr)
    {
      if ((itr->player1 != player) && (itr->bet == bet) && (itr->player2 == EMPTY_PLAYER))
      {
        // нашли открытую игру с нужной ставкой
        games_table_player2.modify(itr, player, [&](auto &g) {
          g.player2 = player;
          g.afksnapshot = eosio::time_point_sec(now());
        });
        return;
      }
      itr++;
    }

    global_table.modify(global_table.begin(), 0, [&](auto &g) {
      g.games++;
    });
    games_table.emplace(player, [&](game &g) {
      g.id = global_table.begin()->games;
      g.player1 = player;
      g.player2 = EMPTY_PLAYER;
      g.commitment1 = EMPTY_CHECKSUM;
      g.commitment2 = EMPTY_CHECKSUM;
      g.afksnapshot = eosio::time_point_sec(now());
      g.bet = bet;
      g.round = 1;
    });
  };
  //@abi action
  void connect(account_name player)
  {
    require_auth(player);
    auto account_row = accounts_table.find(player);
    eosio_assert(account_row == accounts_table.end(), "already registered");
    accounts_table.emplace(player, [&](account &a) {
      a.player = player;
      for (auto i = 0; i < MAX_GAMES; i++)
      {
        a.lastgames.push_back(gameresult{0, 0});
      }
    });
  }
  // eosio.token transfer handler
  void transfer(const account_name from, const account_name to, asset quantity, const std::string memo)
  {
    require_auth(from);
    if (from == _self)
    {
      // вывод токенов - просто разрещаем (В будующем надо добавить еще логики)
      return;
    }

    auto account_row = accounts_table.find(from);
    eosio_assert(account_row != accounts_table.end(), "player shoud register");
    eosio_assert(account_row->deposit.amount == 0, "shoud be only one deposit");
    eosio_assert(account_row->openedgames < MAX_GAMES, "riached max opened games");
    // считаем что пришла сумма с половиной комиссии
    quantity.amount = quantity.amount * TAX_DENOMINATOR / (TAX_DENOMINATOR + TAX_NUMERATOR);
    assert_bet(quantity);
    accounts_table.modify(account_row, 0, [&](account &a) {
      a.deposit = quantity;
    });
    // startGame(from, quantity);
  }
  // tmp function for developing
  void cleartables()
  {
    require_auth(_self);

    for (auto itr = games_table.begin(); itr != games_table.end();)
    {
      itr = games_table.erase(itr);
    }
    for (auto itr = accounts_table.begin(); itr != accounts_table.end();)
    {
      itr = accounts_table.erase(itr);
    }
    /*
    for (auto itr = global_table.begin(); itr != global_table.end();)
    {
      itr = global_table.erase(itr);
    }
    */
  }

  // tmp function for developing
  void inittables()
  {
    require_auth(_self);
    if (global_table.begin() == global_table.end())
    {
      global_table.emplace(_self, [&](global &g) {
        g.games = 0;
      });
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
EOSIO_ABI(rps, (transfer)(commitmove)(revealmove)(cancelgame)(claimexpired)(cleartables)(inittables)(connect)(startgame))
