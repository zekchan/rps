import Eos from 'eosjs-api';
import randomize from 'randomatic';
import child_process from 'child_process'
import nodeInterfaceFabric from './nodeInterfaceFabric';


const ALICE_PUBLIC = 'EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA'
const nodeInterface = nodeInterfaceFabric('http://localhost:8888')
const eos = nodeInterface.eos;
export default eos;
export const cleos = nodeInterface.cleos;

export function getAccountName() {
  return randomize('?', 12, { chars: '12345abcdefghijklmnopqrstuvwxyz' });
}


export async function deployContract() {
  const accountName = getAccountName();

  await cleos(`create account eosio ${accountName} ${ALICE_PUBLIC} ${ALICE_PUBLIC}`)
  await cleos(`set contract ${accountName} /work/eosProjects/rps/rps -p ${accountName}`)
  await cleos(`set account permission ${accountName} active '{"threshold":1, "keys":[{"key":"${ALICE_PUBLIC}", "weight":1}], "accounts": [{"permission":{"actor":"${accountName}","permission":"eosio.code"},"weight":1}]}' owner -p ${accountName}`)
  expect((await eos.getAccount(accountName)).permissions).toEqual([
    {
      "perm_name": "active",
      "parent": "owner",
      "required_auth": {
        "threshold": 1,
        "keys": [
          {
            "key": ALICE_PUBLIC,
            "weight": 1
          }
        ],
        "accounts": [
          {
            "permission": {
              "actor": accountName,
              "permission": "eosio.code"
            },
            "weight": 1
          }
        ],
        "waits": []
      }
    },
    {
      "perm_name": "owner",
      "parent": "",
      "required_auth": {
        "threshold": 1,
        "keys": [
          {
            "key": ALICE_PUBLIC,
            "weight": 1
          }
        ],
        "accounts": [],
        "waits": []
      }
    }
  ])
  return accountName;
}

