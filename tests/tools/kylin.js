import nodeInterfaceFabric from "./nodeInterfaceFabric";
import { getAccountName } from './eos'

const nodeInterface = nodeInterfaceFabric('https://api.kylin-testnet.eospace.io')
const eos = nodeInterface.eos;
export default eos;
export const cleos = nodeInterface.cleos;
const PUBLIC = 'EOS7HNYi6PtR9Fcz8VkovgzDfC48ReXEeXCzfxuR6oxBYfETSUNQ2'
const players = ['fytelawqyhum',
  'edelrxwpjy4f',
  'rst4yp5jdga4',
  '2j1rngekj43e',
  'oedan53bkhto',
  'f1hvtmzdo15h',
  'gx1brarwwfqw',
  '1ww5ll5fwzec',
  'fgl4upzfbxsp',
  'evvxp4ggwngf'];
let idx = 0;
export async function getPlayer() {
  return players[idx++]
  const accountName = getAccountName();
  try {
    await cleos(`system newaccount rpsfaucet111 ${accountName} ${PUBLIC} ${PUBLIC}  -p rpsfaucet111@active  --stake-net '1.0000 EOS' --stake-cpu '3.0000 EOS' --buy-ram '2.0000 EOS' --max-cpu-usage-ms 0`)
  } catch (e) {
    console.log(e)
    return await getPlayer()
  }

  await cleos(`push action eosio.token transfer '[ "rpsfaucet111", "${accountName}", "40.0000 EOS", "" ]' -p rpsfaucet111`)
  return accountName;
}
