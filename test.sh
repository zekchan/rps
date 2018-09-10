#!/usr/bin/env bash

./deployContract.sh
./testscenarioAliceWins.sh
./testscenarioBobWins.sh
./testscenarioIlyaWins.sh
./testscenarioDraw.sh
./testscenarioCancelgame.sh
./testscenarioClaimexperied.sh