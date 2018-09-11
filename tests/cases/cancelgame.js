import eos, { deployContract, getPlayer } from "../tools/eos";
import { gamesTable } from "../tools/helpers";
jest.setTimeout(99999999);

describe('Game cancel', () => {
  let contract, player1, player2;
  beforeAll(async () => {
    ([contract, player1, player2] = await Promise.all([deployContract(), getPlayer(), getPlayer()]))
  })
  test('player can cancel game if no second player', async () => {
    await eos.transfer(player1, contract, '1.0000 EOS', '')
    expect(await gamesTable(contract)).toEqual(
      expect.arrayContaining([
        {
          id: 0,
          player1,
          player2: contract,
          bet: '1.0000 EOS',
          commitment1: '0000000000000000000000000000000000000000000000000000000000000000',
          commitment2: '0000000000000000000000000000000000000000000000000000000000000000',
          fight1: '',
          fight2: '',
          lastseen1: expect.any(String),
          lastseen2: expect.any(String),
          round: 1
        }
      ])
    )
    await eos.transaction(
      {
        actions: [
          {
            account: contract,
            name: 'cancelgame',
            authorization: [{
              actor: player1,
              permission: 'active'
            }],
            data: { player: player1, gameid: 0 }
          }
        ]
      }
    )
    expect(await gamesTable(contract)).toEqual([])
  })
  test('any player cant cancel game ', async () => {
    const gamesRows = expect.arrayContaining([
      {
        id: 0,
        player1,
        player2: contract,
        bet: '2.0000 EOS',
        commitment1: '0000000000000000000000000000000000000000000000000000000000000000',
        commitment2: '0000000000000000000000000000000000000000000000000000000000000000',
        fight1: '',
        fight2: '',
        lastseen1: expect.any(String),
        lastseen2: expect.any(String),
        round: 1
      }
    ]);
    await eos.transfer(player1, contract, '2.0000 EOS', '')
    try {
      await eos.transaction(
        {
          actions: [
            {
              account: contract,
              name: 'cancelgame',
              authorization: [{
                actor: 'alice',
                permission: 'active'
              }],
              data: { player: player1, gameid: 0 }
            }
          ]
        }
      )
      expect(true).toBe(false);
    } catch (e) {
      expect(await gamesTable(contract)).toEqual(
        gamesRows
      )
    }
    try {
      await eos.transaction(
        {
          actions: [
            {
              account: contract,
              name: 'cancelgame',
              authorization: [{
                actor: 'alice',
                permission: 'active'
              }],
              data: { player: 'alice', gameid: 0 }
            }
          ]
        }
      )
      expect(true).toBe(false);
    } catch (e) {
      expect(await gamesTable(contract)).toEqual(
        gamesRows
      )
    }
  })
})