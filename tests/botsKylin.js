import workerFarm from 'worker-farm'
import { getPlayer } from "./tools/kylin";

const BOTS_COUNT = 10;
const workers = workerFarm(require.resolve('./startBot'))

async function start() {
  // await clearAllStartedGames()
  // await cleos('wallet create --to-console')
  const botsNames = await Promise.all(Array.from(Array(BOTS_COUNT)).map(getPlayer))

  console.log('players: ', botsNames)

  botsNames.forEach(player => {
    workers({ player, contract: 'rpsrpsrpsrps' }, () => { })
  })

}

start()