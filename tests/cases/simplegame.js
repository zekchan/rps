import eos, { deployContract } from "../tools/eos";
import randomstring from 'randomstring';
// think twice about using this lib in production
import sha256 from 'sha256'

jest.setTimeout(99999999);
describe('Simple game Alise and Bob', () => {
  describe('HAPPY PATHS', () => {
    let contract, name;
    beforeEach(async () => {
      ({ contract, name } = await deployContract())
    })
    test('Alice plays with bob and win', async () => {
      await eos.transfer('alice', name, '1.0000 EOS', '')
      const gameBase = {
        id: 0,
        player1: 'alice',
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
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([
        gameBase
      ]);
      expect(await eos.getCurrencyBalance('eosio.token', name, 'EOS')).toEqual(['1.0000 EOS'])
      await eos.transfer('bob', name, '1.0000 EOS', '')
      gameBase.player2 = 'bob'
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([
        gameBase
      ]);
      expect(await eos.getCurrencyBalance('eosio.token', name, 'EOS')).toEqual(['2.0000 EOS'])
      const aliceFight = '111'
      const aliceSecret = randomstring.generate();
      const aliceCommittment = sha256(aliceFight + aliceSecret)
      const bobFight = '222'
      const bobSecret = randomstring.generate();
      const bobCommittment = sha256(bobFight + bobSecret)
      await eos.transaction(
        {
          actions: [
            {
              account: name,
              name: 'commitmove',
              authorization: [{
                actor: 'alice',
                permission: 'active'
              }],
              data: { player: 'alice', gameid: 0, commitment: aliceCommittment }
            }
          ]
        }
      )
      gameBase.commitment1 = aliceCommittment
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([
        gameBase
      ]);
      await eos.transaction(
        {
          actions: [
            {
              account: name,
              name: 'commitmove',
              authorization: [{
                actor: 'bob',
                permission: 'active'
              }],
              data: { player: 'bob', gameid: 0, commitment: bobCommittment }
            }
          ]
        }
      )
      gameBase.commitment2 = bobCommittment
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([
        gameBase
      ]);
      await eos.transaction(
        {
          actions: [
            {
              account: name,
              name: 'revealmove',
              authorization: [{
                actor: 'alice',
                permission: 'active'
              }],
              data: { player: 'alice', gameid: 0, fight: aliceFight, secret: aliceSecret }
            }
          ]
        }
      )
      gameBase.fight1 = aliceFight
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([
        gameBase
      ]);
      await eos.transaction(
        {
          actions: [
            {
              account: name,
              name: 'revealmove',
              authorization: [{
                actor: 'bob',
                permission: 'active'
              }],
              data: { player: 'bob', gameid: 0, fight: bobFight, secret: bobSecret }
            }
          ]
        }
      )
      expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([]);
      expect(await eos.getCurrencyBalance('eosio.token', name, 'EOS')).toEqual(['0.0400 EOS'])
    })
  })
})
