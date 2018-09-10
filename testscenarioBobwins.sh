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
# bob - move = "111"; secret = "asdasdasd"; sha256(move + secret) = "b0316749a5ea496b29631515fa3ec9c13554406f238ff609f666fa7f86fdf10b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "b0316749a5ea496b29631515fa3ec9c13554406f238ff609f666fa7f86fdf10b"]' -p bob
cleos get table rps rps games
sleep 0.4
# alice - move = "424"; secret = "sdfsskdjf"; sha256(move + secret) = "311ef1fa21b4c98a8e5ea7fb755a5817a6242ac0a755c49fc05850942ddeccd8"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "311ef1fa21b4c98a8e5ea7fb755a5817a6242ac0a755c49fc05850942ddeccd8"]' -p alice
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, "111", "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, "424", "sdfsskdjf"]' -p alice
cleos get table rps rps games
balance bob
balance alice
balance rps
stats