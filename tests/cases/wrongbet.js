import { deployContract, getPlayer, cleos } from "../tools/eos";
jest.setTimeout(99999999);

describe('cannot create game with bet not includet to whitelist', () => {
  test('player1 get score', async () => {
    const [contract, player] = await Promise.all([deployContract(), getPlayer()])
    try {
      await cleos(`push action eosio.token transfer '[ "${player}","${contract}", "0.5400 EOS", "" ]' -p ${player}`)
      expect(true).toBe(false)
    } catch (e) {
      expect(e).toBeDefined()
    }
  })
})