import workerFarm from 'worker-farm'
import { getAccountName } from "./tools/eos";
import { cleos } from "./kylin";
import fetch from 'node-fetch'
import { sleep } from './tools/helpers';

const BOTS_COUNT = 5;
const workers = workerFarm(require.resolve('./startBot'))
async function getPlayer() {
  try {
    const name = getAccountName()
    let resp = await fetch(`http://faucet.cryptokylin.io/create_account?${name}`)
    let body = await resp.json()
    if (body.msg !== 'succeeded') {
      throw new Error('bad name')
    }
    console.log('created bot ', name)
    await cleos(`wallet import --private-key ${body.keys.active_key.private}`)
    await cleos(`wallet import --private-key ${body.keys.owner_key.private}`)
    await sleep(1000)
    console.log('gettings token for... ', name)
    await cleos(`transfer rpsrpsrpsrps ${name} '20.000 EOS' '' -p rpsrpsrpsrps`)
    await cleos(`system delegatebw rpsrpsrpsrps ${name} '5.000 EOS' '5.000 EOS' -p rpsrpsrpsrps`)
    console.log('staking eos for', name)
    await sleep(1000)
    await cleos(`system delegatebw rpsrpsrpsrps ${name} '5.000 EOS' '5.000 EOS' -p rpsrpsrpsrps`)
    console.log('buing ram for', name)
    await sleep(1000)
    await cleos(`system buyram rpsrpsrpsrps ${name} '1.000 EOS' -p rpsrpsrpsrps`)
    console.log('bot ', name, 'ready!')
    return name
  } catch (e) {
    console.log(e)
    return await getPlayer()
  }
}
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