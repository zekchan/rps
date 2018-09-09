#!/usr/bin/env bash
function balance() {
  echo "$@ balance:"
  cleos get currency balance eosio.token "$@"
}
function unlockwallet() {
  cleos wallet unlock -n default --password PW5J5jYhgViyJmfEAjYpVsBKQYKrYxnzLfprpRysmKG6o1oZNMbPv
}

function stats() {
  cleos get table rps rps accounts
}

unlockwallet