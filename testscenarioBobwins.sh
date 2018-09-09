#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup
source ./helpers.sh
echo "*****************ALISE WINS TEST *****************"
sleep 0.4
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE CONNECTED TO GAME"
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", "join:0"]' -p alice
cleos get table rps rps games
balance bob
balance alice
balance rps
sleep 0.4
# bob - move = 1; secret = "asdasdasd"; sha256(move + secret) = "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"]' -p bob
cleos get table rps rps games
sleep 0.4
# alice - move = 4; secret = "sdfsskdjf"; sha256(move + secret) = "deb6182bae9b88a6b6bf11a38c10a9c97ec71886e96f3b4a1feb1ffc09a87616"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "deb6182bae9b88a6b6bf11a38c10a9c97ec71886e96f3b4a1feb1ffc09a87616"]' -p alice
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, 1, "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, 4, "sdfsskdjf"]' -p alice
cleos get table rps rps games
balance bob
balance alice
balance rps
stats