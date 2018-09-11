import eos, { deployContract, getPlayer } from './eos'
import randomstring from 'randomstring';
// think twice about using this lib in production
import sha256 from 'sha256'

export async function balance(account) {
  return (await eos.getCurrencyBalance('eosio.token', account, 'EOS'))[0]
}
export async function gamesTable(account) {
  return (await eos.getTableRows(true, account, account, 'games')).rows
}
export async function accountsTable(account) {
  return (await eos.getTableRows(true, account, account, 'accounts')).rows
}

export async function playgame(player1, player2, fight1, fight2, name, checkbalance = true) {
  await eos.transfer(player1, name, '1.0000 EOS', '')
  const gameBase = {
    id: 0,
    player1,
    player2: name,
    bet: '1.0000 EOS',
    commitment1: '0000000000000000000000000000000000000000000000000000000000000000',
    commitment2: '0000000000000000000000000000000000000000000000000000000000000000',
    fight1: '',
    fight2: '',
    lastseen1: expect.any(String),
    lastseen2: expect.any(String),
    round: 1
  }
  expect(await gamesTable(name)).toEqual([
    gameBase
  ]);
  checkbalance && expect(await balance(name)).toEqual('1.0000 EOS')
  await eos.transfer(player2, name, '1.0000 EOS', '')
  gameBase.player2 = player2
  expect(await gamesTable(name)).toEqual([
    gameBase
  ]);

  checkbalance && expect(await balance(name)).toEqual('2.0000 EOS')
  const secret1 = randomstring.generate();
  const committment1 = sha256(fight1 + secret1)
  const secret2 = randomstring.generate();
  const committment2 = sha256(fight2 + secret2)
  await eos.transaction(
    {
      actions: [
        {
          account: name,
          name: 'commitmove',
          authorization: [{
            actor: player1,
            permission: 'active'
          }],
          data: { player: player1, gameid: 0, commitment: committment1 }
        }
      ]
    }
  )
  gameBase.commitment1 = committment1
  expect(await gamesTable(name)).toEqual([
    gameBase
  ]);

  await eos.transaction(
    {
      actions: [
        {
          account: name,
          name: 'commitmove',
          authorization: [{
            actor: player2,
            permission: 'active'
          }],
          data: { player: player2, gameid: 0, commitment: committment2 }
        }
      ]
    }
  )
  gameBase.commitment2 = committment2


  expect(await gamesTable(name)).toEqual([
    gameBase
  ]);
  await eos.transaction(
    {
      actions: [
        {
          account: name,
          name: 'revealmove',
          authorization: [{
            actor: player1,
            permission: 'active'
          }],
          data: { player: player1, gameid: 0, fight: fight1, secret: secret1 }
        }
      ]
    }
  )

  gameBase.fight1 = fight1
  expect(await gamesTable(name)).toEqual([
    gameBase
  ]);
  await eos.transaction(
    {
      actions: [
        {
          account: name,
          name: 'revealmove',
          authorization: [{
            actor: player2,
            permission: 'active'
          }],
          data: { player: player2, gameid: 0, fight: fight2, secret: secret2 }
        }
      ]
    }
  )
  expect(await gamesTable(name)).toEqual([]);
}

export async function playCombination(fight1, fight2, winner = 1) {
  const [contract, player1, player2] = await Promise.all([deployContract(), getPlayer(), getPlayer()])
  await playgame(player1, player2, fight1, fight2, contract)
  const winplayer = winner === 1 ? player1 : player2
  const looseplayer = winner === 1 ? player2 : player1
  expect(await balance(winplayer)).toBe('1000.9600 EOS')
  expect(await balance(looseplayer)).toBe('999.0000 EOS')
  expect(await balance(contract)).toBe('0.0400 EOS')
  const accounts = await accountsTable(contract)
  expect(accounts).toEqual(
    expect.arrayContaining([
      {
        player: winplayer,
        games: 1,
        winstreak: 1,
        wins: 1,
        score: 1
      }
    ])
  )
  expect(accounts).toEqual(
    expect.arrayContaining([
      {
        player: looseplayer,
        games: 1,
        wins: 0,
        winstreak: 0,
        score: 0
      }
    ])
  )
}