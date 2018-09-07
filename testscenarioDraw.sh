#!/usr/bin/env bash

# using https://github.com/zekchan/eos-local-env-setup setup

echo "BOB CREATED GAME"
cleos push action eosio.token transfer '["bob", "rps", "1.0000 EOS", "create:"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE CONNECTED TO GAME"
cleos push action eosio.token transfer '["alice", "rps", "1.0000 EOS", "join:0"]' -p alice
cleos get table rps rps games
sleep 0.4
# bob - move = 1; secret = "asdasdasd"; sha256(move + secret) = "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"]' -p bob
cleos get table rps rps games
sleep 0.4
# alice - move = 1; secret = "2ded2ded2ed2ed"; sha256(move + secret) = "1a5a2a879f57773b23f0ce0d64aebd082e2d13b87696f79de67b21ea97763f7c"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "1a5a2a879f57773b23f0ce0d64aebd082e2d13b87696f79de67b21ea97763f7c"]' -p alice
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, 1, "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, 1, "2ded2ded2ed2ed"]' -p alice
echo "DRAW GAME"
cleos get table rps rps games
sleep 0.4
# тут должна быть ничья и опять "пустая игра" Запускаем aliseWins заново
# bob - move = 1; secret = "asdasdasd"; sha256(move + secret) = "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"
echo "BOB COMITTED MOVE"
cleos push action rps commitmove '["bob", 0, "971bffc5e741ecbc9beef4a8e00c7dd9aa7b8c8af6ffc5198bc2c52f3f1e455b"]' -p bob
cleos get table rps rps games
sleep 0.4
# alice - move = 2; secret = "sdfsskdjf"; sha256(move + secret) = "55f079f8d46a2dab2fe4441f094d502bf3b0194dbb259d996dd41be7c4cc1a0d"
echo "ALISE COMITTED MOVE"
cleos push action rps commitmove '["alice", 0, "55f079f8d46a2dab2fe4441f094d502bf3b0194dbb259d996dd41be7c4cc1a0d"]' -p alice
cleos get table rps rps games
sleep 0.4
echo "BOB REVEALED HIS MOVE"
cleos push action rps revealmove '["bob", 0, 1, "asdasdasd"]' -p bob
cleos get table rps rps games
sleep 0.4
echo "ALICE REVEALED HER MOVE"
cleos push action rps revealmove '["alice", 0, 2, "sdfsskdjf"]' -p alice
cleos get table rps rps games
sleep 0.4