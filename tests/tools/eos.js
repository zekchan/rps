import Eos from 'eosjs';
import path from 'path';
import fs from 'fs';
import randomize from 'randomatic';

const ALICE_PUBLIC = 'EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA'
const keys = [
  '5HrCoHT55VdetZp1mdih9UJ4cUftd1JbGsTQTZU14VhdHWKy92B', // alice EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA
  '5KGVkBbyrpDoZrF4RGAV48N5xKxYCcLBUuhZokbvg9czcqxTrPo', // bob
  '5JMcyQ1VeZUjJ5HgBHjUGHLh6pWdr1UbPSgLETimkXBFW3Trh5a', // eosio token
  '5JBgxSG8aSMWik8ZCey2trACqCCVLute7Y7k5Y1CYdJSo3CYvY9', // ilya
  '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3', // eosio,
  '5Jmsawgsp1tQ3GD6JyGCwy1dcvqKZgX6ugMVMdjirx85iv5VyPR'
]
const eos = Eos({
  // verbose: true, debug: false,
  keyProvider: keys,// private key 
  httpEndpoint: 'http://127.0.0.1:8888',
  chainId: 'cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f',
});

export default eos;


function getAccountName() {
  return randomize('a', 12, {chars: 'jonschlinkert'});
}

const wasm = fs.readFileSync(path.resolve('../rps/rps.wasm'));
const abi = fs.readFileSync(path.resolve('../rps/rps.abi'));


export async function deployContract() {
  const accountName = getAccountName();

  await eos.transaction(tr => {
    tr.newaccount({
      creator: 'alice',
      name: accountName,
      owner: ALICE_PUBLIC,
      active: ALICE_PUBLIC
    })
  })
  await eos.transaction(tr => {
    tr.setcode(accountName, 0, 0, wasm)
    tr.setabi(accountName, JSON.parse(abi))
  })
  await eos.transaction(tr => {
    tr.updateauth({
      account: accountName,
      permission: `active`,
      parent: 'owner',
      auth: {
        "threshold": 1,
        "keys": [{ "key": ALICE_PUBLIC, "weight": 1 }],
        "accounts": [{ "permission": { "actor": accountName, "permission": "eosio.code" }, "weight": 1 }]
      }
    });
  })
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
  await eos.newaccount({
    creator: 'alice',
    name: accountName,
    owner: ALICE_PUBLIC,
    active: ALICE_PUBLIC
  })
  await eos.transaction(
    {
      actions: [
        {
          account: 'eosio.token',
          name: 'issue',
          authorization: [{
            actor: 'eosio',
            permission: 'active'
          }],
          data: { to: accountName, quantity: '1000.0000 EOS', memo: 'creating account' }
        }
      ]
    }
  )
  return accountName;
}