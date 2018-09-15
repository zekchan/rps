import eos, { cleos } from './tools/kylin'
import randomize from 'randomatic';
import sha256 from 'sha256'

const bets = [1, 2].map(eos => `${eos}.0000 EOS`);

function getBet() {
  // пока все играют на одну ставку
  // return `1.0000 EOS`
  return bets[Math.floor(Math.random() * bets.length)];
}
async function sleep(delay = 1000) {
  return new Promise(resolve => setTimeout(resolve, delay))
}

async function whilePass(fn) {
  try {
    return await fn()
  } catch (e) {
    return await whilePass(fn)
  }
}
async function getPlayersGame(player, contract) {
  return whilePass(async () => {
    await sleep()
    const table = await eos.getTableRows(true, contract, contract, 'games', undefined, 0, -1, 500)
    const game = table.rows.find(row => (
      ((row.player1 == player) || (row.player2 == player)) && (row.player2 !== contract)
    ))
    if (game) {
      return game;
    }
    return await getPlayersGame(player, contract)
  })
}
const EMPTY = "0000000000000000000000000000000000000000000000000000000000000000"
async function bothPlayersCommitted(id, contract) {
  return whilePass(async () => {
    await sleep()
    // ищем игру с игроком player на второй позиции
    let { rows: [game] } = await eos.getTableRows(true, contract, contract, 'games', undefined, id, -1, 1, 'i64', 1)
    if (game.commitment1 !== EMPTY && game.commitment2 !== EMPTY) {
      return;
    }
    return await bothPlayersCommitted(id)
  })
}
async function gameFinished(_game, contract) {
  return whilePass(async () => {
    await sleep()
    const table = await eos.getTableRows(true, contract, contract, 'games', undefined, 0, -1, 500)
    console.log('GAMES: gameFinished ', table.rows.length)
    const game = table.rows.find(row => (row.player1 == _game.player1) && (row.player2 == _game.player2))
    if (!game) {
      return; // игра кончилась
    }
    if (game.commitment1 == EMPTY || game.commitment2 == EMPTY) {
      // ничья
      return game;
    }
    return await gameFinished(_game) // еще противник не ответил, ждем статус игры
  })
}
async function playSomeGame(player, contract = 'rps') {
  // get bet randomly
  const bet = getBet()
  console.log(`player ${player} wonts to play on ${bet}`)
  await whilePass(
    () => cleos(`push action eosio.token transfer '[ "${player}","${contract}", "${bet}", "" ]' -p ${player}`)
  )
  console.log(`player ${player} waiting for second gamer`)
  let game = await getPlayersGame(player, contract)

  do {
    console.log(player, `start playing game ${game.id} player1: ${game.player1} player2: ${game.player2}`)
    const fight = randomize('?', 3, { chars: '12345' })
    const secret = randomize('Aa0', 10)
    const commitment = sha256(fight + secret)
    await whilePass(
      () => cleos(`push action ${contract} commitmove '["${player}", ${game.id}, "${commitment}"]' -p ${player}`)
    )
    console.log(player, ' commited move ', fight, ' in game ', game.id)
    await bothPlayersCommitted(game.id, contract);
    console.log(player, ' both players commited game ', game.id)
    await whilePass(
      () => cleos(`push action ${contract} revealmove '["${player}", ${game.id}, "${fight}", "${secret}"]' -p ${player}`)
    )
    console.log(player, ' revealed move gameid: ', game.id)
    game = await gameFinished(game, contract)
    if (!game) {
      return;
    }
    console.log(player, 'DRAW GAME ', game.id)
  } while (game);
}
module.exports = async ({ player, contract }, callback) => {
  while (true) {
    await playSomeGame(player, contract)
  }
}