import workerFarm from 'worker-farm'
import eos, { cleos, getAccountName } from "./tools/eos";
import fetch from 'node-fetch'

const BOTS_COUNT = 7;
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
async function getPlayer() {
  const name = getAccountName()
  const resp = await fetch(`http://faucet.cryptokylin.io/create_account?${name}`)
  const body = await resp.json
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