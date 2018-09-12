import eos, { cleos } from './tools/eos'

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
  // ищем игру с игроком player на второй позиции
  let table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, player, -1, 50, 'i64', 3)
  if (table.rows.length === 0) {
    // если не нашли -  на первой позиции
    table = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, player, -1, 50, 'i64', 2)
  }

  return table.rows.find(row => ((row.player1 == player) || (row.player2 == player)))
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
  } while ((!game || (game.player1 === 'rps') || (game.player2 === 'rps')))
  if (((game.player1 === 'rps') || (game.player2 === 'rps'))) {
    await cleos(`push action rps cancelgame '["${player}", ${game.id}]' -p ${player}`)
    console.log(player, `canceled game ${game.id}`);
    return;
  }

  console.log(player, `start playing game ${game.id} player1: ${game.player1} player2: ${game.player2}`)
}
module.exports = async (player, callback) => {
  // while (true) {
  await playSomeGame(player)
  // }
}