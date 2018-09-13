import { getPlayer } from "./tools/eos";
import workerFarm from 'worker-farm'
import eos, { cleos } from "./tools/eos";

const BOTS_COUNT = 5;
const workers = workerFarm(require.resolve('./startBot'))
async function clearAllStartedGames() {
  const { rows } = await eos.getTableRows(true, 'rps', 'rps', 'games', undefined, 0, -1, 500)
  let found = 0;
  for (let i = 0; i < rows.length; i++) {
    if (rows[i].player2 === 'rps') {
      await cleos(`push action rps cancelgame '["${rows[i].player1}", ${rows[i].id}]' -p ${rows[i].player1}`)
      found++;
    }
  }
  console.log('started games found: ', found)
}
async function start() {
  await clearAllStartedGames()
  const botsNames = await Promise.all(Array.from(Array(BOTS_COUNT)).map(getPlayer))
  console.log('players: ', botsNames)
  botsNames.forEach(player => {
    workers(player, () => { })
  })
}

start()