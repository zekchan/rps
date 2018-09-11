import { deployContract, getPlayer } from "../../tools/eos";
import { playgame, balance } from "../../tools/helpers";
jest.setTimeout(99999999);

test('player1 wins player2 221vs111', async () => {
  const [contract, player1, player2] = await Promise.all([deployContract(), getPlayer(), getPlayer()])
  await playgame(player1, player2, '221', '111', contract)
  expect(await balance(player1)).toBe('1000.9600 EOS')
  expect(await balance(player2)).toBe('999.0000 EOS')
  expect(await balance(contract)).toBe('0.0400 EOS')
})
