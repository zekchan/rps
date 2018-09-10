import Eos from 'eosjs';

const keys = [
  '5HrCoHT55VdetZp1mdih9UJ4cUftd1JbGsTQTZU14VhdHWKy92B', // alice
  '5KGVkBbyrpDoZrF4RGAV48N5xKxYCcLBUuhZokbvg9czcqxTrPo', // bob
  '5JMcyQ1VeZUjJ5HgBHjUGHLh6pWdr1UbPSgLETimkXBFW3Trh5a', // eosio token
  '5JBgxSG8aSMWik8ZCey2trACqCCVLute7Y7k5Y1CYdJSo3CYvY9', // ilya
]
const eos = Eos({
  keyProvider: '5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3',// private key
  httpEndpoint: 'http://127.0.0.1:8888',
  chainId: chain.sys,
});