#!/usr/bin/env bash
source ./helpers.sh
echo "Compiling contract"
eosiocpp -o ./rps/rps.wasm ./rps/rps.cpp
cleos create account eosio rps EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF EOS88PUwFsK1wYAiQEqEhUAGwXyv1hCcj4XPuCkvEyJ6sbtsxELiF
cleos set contract rps ./rps -p rps
# ставим rps@eosio.code пермисию над rps@active чтобы когнтракт мог снять с себя бабки
cleos set account permission rpsrpsrpsrps active '{"threshold":1, "keys":[{"key":"EOS5DwzYjyMDm3geRYLWfX1Hk8hq1iWpJ1x8YBkCNggHnfe39xK1N", "weight":1}], "accounts": [{"permission":{"actor":"rpsrpsrpsrps","permission":"eosio.code"},"weight":1}]}' owner -p rpsrpsrpsrps