#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup

source ./helpers.sh
echo "*****************CANCELGAME TEST *****************"
sleep 0.4
balance bob
balance rps
echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "5.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
balance bob
balance rps
sleep 0.4
echo "BOB CANELED GAME"
cleos push action rps cancelgame '["bob", 0]' -p bob
cleos get table rps rps games
balance bob
balance rps