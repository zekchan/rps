import eos, { deployContract, getPlayer } from "../tools/eos";
jest.setTimeout(99999999);

describe('cannot create game with bet not includet to whitelist', () => {
  test('player1 get score', async () => {
    const [contract, player] = await Promise.all([deployContract(), getPlayer()])
    try {
      await eos.transfer(player, contract, '0.5400 EOS', '')
      expect(true).toBe(false)
    } catch (e) {
      expect(e).toMatch('Internal Service Error')
    }
  })
})