#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup

echo "Compiling contract"
eosiocpp -o ./rps/rps.wasm ./rps/rps.cpp
cleos create account eosio rps EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF
cleos set contract rps ./rps -p rps
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", "join:0"]' -p alice
cleos get table rps rps games