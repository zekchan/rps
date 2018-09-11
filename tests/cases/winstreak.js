import { playgame, accountsTable } from "../tools/helpers";
import { deployContract, getPlayer } from "../tools/eos";
jest.setTimeout(99999999);

describe('winstreak and rating', () => {
  test('player1 get score', async () => {
    const [contract, player1, player2] = await Promise.all([deployContract(), getPlayer(), getPlayer()])
    await playgame(player1, player2, '222', '111', contract, false)
    await playgame(player2, player1, '111', '222', contract, false)
    await playgame(player2, player1, '111', '222', contract, false)
    let accounts = await accountsTable(contract)
    expect(accounts).toEqual(
      expect.arrayContaining([
        {
          player: player1,
          games: 3,
          winstreak: 3,
          score: 6,
          wins: 3
        }
      ])
    )
    expect(accounts).toEqual(
      expect.arrayContaining([
        {
          player: player2,
          games: 3,
          winstreak: 0,
          score: 0,
          wins: 0
        }
      ])
    )

    await playgame(player1, player2, '111', '222', contract, false)
    accounts = await accountsTable(contract)
    expect(accounts).toEqual(
      expect.arrayContaining([
        {
          player: player1,
          games: 4,
          winstreak: 0,
          score: 6,
          wins: 3
        }
      ])
    )
    expect(accounts).toEqual(
      expect.arrayContaining([
        {
          player: player2,
          games: 4,
          winstreak: 1,
          score: 1,
          wins: 1
        }
      ])
    )
  })
})