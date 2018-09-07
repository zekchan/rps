#!/usr/bin/env bash
function balance() {
  echo "$@ balance:"
  cleos get currency balance eosio.token "$@"
}