import Eos from 'eosjs';
import path from 'path';
import fs from 'fs';

const keys = [
  '5HrCoHT55VdetZp1mdih9UJ4cUftd1JbGsTQTZU14VhdHWKy92B', // alice EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA
  '5KGVkBbyrpDoZrF4RGAV48N5xKxYCcLBUuhZokbvg9czcqxTrPo', // bob
  '5JMcyQ1VeZUjJ5HgBHjUGHLh6pWdr1UbPSgLETimkXBFW3Trh5a', // eosio token
  '5JBgxSG8aSMWik8ZCey2trACqCCVLute7Y7k5Y1CYdJSo3CYvY9', // ilya
]
const eos = Eos({
  keyProvider: keys,// private key 
  httpEndpoint: 'http://127.0.0.1:8888',
  chainId: 'cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f',
});

export default eos;


function getAccountName() {
  var text = "";
  var possible = "abcdefghijklmnopqrstuvwxyz123";
  const length = 1 + Math.floor(Math.random() * 10)
  for (var i = 0; i < length; i++)
    text += possible.charAt(Math.floor(Math.random() * possible.length));

  return text;
}
const wasm = fs.readFileSync(path.resolve('../rps/rps.wasm'));
const abi = fs.readFileSync(path.resolve('../rps/rps.abi'));

export async function deployContract() {
  const accountName = getAccountName();

  await eos.transaction(tr => {
    tr.newaccount({
      creator: 'alice',
      name: accountName,
      owner: 'EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA',
      active: 'EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA'
    })
  })
  await eos.transaction(tr => {
    tr.setcode(accountName, 0, 0, wasm)
    tr.setabi(accountName, JSON.parse(abi))
    tr.updateauth({
      account: accountName,
      permission: `active`,
      parent: 'owner',
      auth: {
        "threshold": 1,
        "keys": [{ "key": "EOS79NjptwuiygNmWmGLXTHoK4eUsZZxWgprQZ3rgDdJvkbLFTkSA", "weight": 1 }],
        "accounts": [{ "permission": { "actor": accountName, "permission": "eosio.code" }, "weight": 1 }]
      }
    },
      {
        authorization: `${accountName}@owner`
      }
    );
  })
  const contract = await eos.contract(accountName)

  return { contract, name: accountName };
}
