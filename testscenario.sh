#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup

echo "Compiling contract"
eosiocpp -o ./rps/rps.wasm ./rps/rps.cpp
cleos create account eosio rps EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF
cleos set contract rps ./rps -p rps
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
echo "ALICE CONNECTED TO GAME"
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", "join:0"]' -p alice
cleos get table rps rps games
# bob - move = 1; secret = "asdasdasd"; sha256(move + secret) = "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"]' -p bob
cleos get table rps rps games
# alice - move = 2; secret = "sdfsskdjf"; sha256(move + secret) = "55f079f8d46a2dab2fe4441f094d502bf3b0194dbb259d996dd41be7c4cc1a0d"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "55f079f8d46a2dab2fe4441f094d502bf3b0194dbb259d996dd41be7c4cc1a0d"]' -p alice
cleos get table rps rps games
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, 1, "asdasdasd"]' -p bob
cleos get table rps rps games
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, 2, "sdfsskdjf"]' -p alice
cleos get table rps rps games