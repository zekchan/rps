#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup
source ./helpers.sh
echo "*****************ALISE WINS TEST *****************"
sleep 0.4
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", ""]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE CONNECTED TO GAME"
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", ""]' -p alice
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
# alice - move = "222"; secret = "sdfsskdjf"; sha256(move + secret) = "fd713023ba64ce8dbe3e707bc7355b44412d7e1e47357248e24fbe055f8217e5"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "fd713023ba64ce8dbe3e707bc7355b44412d7e1e47357248e24fbe055f8217e5"]' -p alice
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, "111", "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, "222", "sdfsskdjf"]' -p alice
cleos get table rps rps games
balance bob
balance alice
balance rps
stats