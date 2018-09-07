#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup
source ./helpers.sh
echo "****************Claim expeired test***************"
balance alise
balance bob
balance rps
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE CONNECTED TO GAME"
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", "join:0"]' -p alice
cleos get table rps rps games
echo "WAILT 3 seconds"
sleep 3
cleos get table rps rps games
# bob - move = 1; secret = "asdasdasd"; sha256(move + secret) = "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"]' -p bob
cleos get table rps rps games
echo "WAIT 2 MINUTES and 2 seconds..."
sleep 122
echo "ALISE AFK SO BOB CLAIMS HIS REWARD"
unlockwallet
cleos push action rps claimexpired '["bob", 0]' -p bob
cleos get table rps rps games
balance alice
balance bob
balance rps