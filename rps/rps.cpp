#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/asset.hpp>
#include <string>

using namespace eosio;

class rps : public eosio::contract
{
public:
  using contract::contract;
  rps(account_name self) : contract(self), games_table(self, self) {}
  static void checkBet(asset bet)
  {
    eosio_assert(bet.symbol == S(4, EOS), "only accepts EOS for deposits");
    eosio_assert(bet.is_valid(), "Invalid token transfer");
    eosio_assert(bet.amount > 0, "Quantity must be positive");
  }
  // @abi table
  struct game
  {
    uint64_t id;
    account_name player1;
    asset bet;
    uint64_t primary_key() const { return id; };

    EOSLIB_SERIALIZE(game, (id)(player1)(bet))
  };

  typedef eosio::multi_index<N(games), game> games_index;

  games_index games_table;

  void startgame(const account_name player1, const asset bet)
  {
    games_table.emplace(_self, [&](game &g) {
      g.id = games_table.available_primary_key();
      g.player1 = player1;
      g.bet = bet;
    });
  };
  // eosio.token transfer handler
  void transfer(const account_name from, const account_name to, const asset& quantity, const std::string memo)
  {
    require_auth(from);
    checkBet(quantity);
    size_t del = memo.find(':');
    auto action = memo.substr(0, del);
    if (action == "create")
    {
      startgame(from, quantity);
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
EOSIO_ABI(rps, (transfer))
