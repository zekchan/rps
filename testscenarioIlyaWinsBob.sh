#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup
source ./helpers.sh
echo "*****************ILYA WINS TEST *****************"
sleep 0.4
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ILYA CONNECTED TO GAME"
cleos push action eosio.token transfer '["ilya", "rps", "1.0000 EOS", "join:0"]' -p ilya
cleos get table rps rps games
balance bob
balance ilya
balance rps
sleep 0.4
# bob - move = "111"; secret = "asdasdasd"; sha256(move + secret) = "b0316749a5ea496b29631515fa3ec9c13554406f238ff609f666fa7f86fdf10b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "b0316749a5ea496b29631515fa3ec9c13554406f238ff609f666fa7f86fdf10b"]' -p bob
cleos get table rps rps games
sleep 0.4
# ilya - move = "222"; secret = "sdfsskdjf"; sha256(move + secret) = "fd713023ba64ce8dbe3e707bc7355b44412d7e1e47357248e24fbe055f8217e5"
echo "ILYA COMITTED MOVE"
cleos push action rps commitmove '["ilya", 0, "fd713023ba64ce8dbe3e707bc7355b44412d7e1e47357248e24fbe055f8217e5"]' -p ilya
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, "111", "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ilya REVEALED HER MOVE"
cleos push action rps revealmove '["ilya", 0, "222", "sdfsskdjf"]' -p ilya
cleos get table rps rps games
balance bob
balance ilya
balance rps
stats