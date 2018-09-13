import eos, { cleos } from './tools/eos'
import equal from 'fast-deep-equal'
import randomize from 'randomatic';
import sha256 from 'sha256'

const bets = [1, 2, 3, 4, 5].map(eos => `${eos}.0000 EOS`);
function getBet() {
  // пока все играют на одну ставку
  return `1.0000 EOS`
  // return bets[Math.floor(Math.random() * bets.length)];
}
async function sleep(delay = 300) {
  return new Promise(resolve => setTimeout(resolve, delay))
}
async function getPlayersGame(player) {
  /*
  // ищем игру с игроком player на второй позиции
  let table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, player, -1, 50, 'i64', 3)
  if (table.rows.length === 0) {
    // если не нашли -  на первой позиции
    table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, player, -1, 50, 'i64', 2)
  }
  */
  // пока ищем нужную игру перебором всех
  const table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, 0, -1, 500)
  let count = 0;
  table.rows.forEach(row => {
    if (row.player1 == player || row.player2 == player) {
      count++
    }
  });
  console.log('FOUND ', count, ' GAMES')
  return table.rows.find(row => ((row.player1 == player) || (row.player2 == player)))
}
const EMPTY = "0000000000000000000000000000000000000000000000000000000000000000"
async function bothPlayersCommitted(id) {
  // ищем игру с игроком player на второй позиции
  let { rows: [game] } = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, id, -1, 1, 'i64', 1)

  if (game.commitment1 !== EMPTY && game.commitment2 !== EMPTY) {
    return;
  }
  return await bothPlayersCommitted(id)
}
async function gameFinished(_game) {
  const table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, 0, -1, 500)
  const game = table.rows.find(row => (row.player1 == _game.player1) && (row.player2 == _game.player2))
  if (!game) {
    return; // игра кончилась
  }
  if (game.commitment1 == EMPTY && game.commitment2 == EMPTY) {
    // ничья
    return game;
  }
  return await gameFinished(_game) // еще противник не ответил, ждем статус игры
}

async function playSomeGame(player) {
  // get bet randomly
  const bet = getBet()
  console.log(`player ${player} wonts to play on ${bet}`)
  await cleos(`push action eosio.token transfer '[ "${player}","rps", "${bet}", "" ]' -p ${player}`)
  console.log(`player ${player} waiting for second gamer`)
  let game;
  do {
    await sleep()
    game = await getPlayersGame(player)
    // ждем пока к игре кто-то подключится 
    // делаем только пять попыток, потом отменяем игру и уходим на следубщий круг
  } while ((!game || (game.player2 === 'rps')))

  do {
    console.log(player, `start playing game ${game.id} player1: ${game.player1} player2: ${game.player2}`)
    const fight = randomize('?', 3, { chars: '123' })
    const secret = randomize('Aa0', 10)
    const commitment = sha256(fight + secret)
    await cleos(`push action rps commitmove '["${player}", ${game.id}, "${commitment}"]' -p ${player}`)
    console.log(player, ' commited move ', fight, ' in game ', game.id)
    await bothPlayersCommitted(game.id);
    console.log(player, ' both players commited game ', game.id)
    await cleos(`push action rps revealmove '["${player}", ${game.id}, "${fight}", "${secret}"]' -p ${player}`)
    console.log(player, ' revealed move gameid: ', game.id)
    game = await gameFinished(game)
    if (!game) {
      return;
    }
    console.log(player, 'DRAW GAME ', game.id)
  } while (game);
}
module.exports = async (player, callback) => {
  while (true) {
    await playSomeGame(player)
  }
}