import Eos from 'eosjs-api';
import randomize from 'randomatic';
import child_process from 'child_process'


const ALICE_PUBLIC = 'EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA'

const eos = Eos({
  httpEndpoint: 'http://127.0.0.1:8888',
})

export default eos;


function getAccountName() {
  return randomize('?', 12, { chars: '12345abcdefghijklmnopqrstuvwxyz' });
}
const exec = script => new Promise((resolve, reject) => {
  child_process.exec(script, err => {
    if (err) {
      reject(err)
    } else {
      resolve()
    }
  })
})
const sleep = (delay) => new Promise(resolve => setTimeout(resolve, delay))
export async function cleos(args, attemp = 0) {
  try {
    await exec(`docker exec -i eosio /opt/eosio/bin/cleos -u http://localhost:8888 ${args}`)
  } catch (e) {
    if (e.message.indexOf('Transaction took too long') === -1) {
      throw e
    } else {
      await cleos(args, attemp + 1)
    }
  }
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

export async function getPlayer() {
  const accountName = getAccountName();
  await cleos(`create account eosio ${accountName} ${ALICE_PUBLIC} ${ALICE_PUBLIC}`)
  await cleos(`push action eosio.token issue '[ "${accountName}", "1000.0000 EOS", "" ]' -p eosio`)
  return accountName;
}