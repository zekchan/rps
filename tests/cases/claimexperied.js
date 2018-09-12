import { deployContract, getPlayer, cleos } from "../tools/eos";
import { gamesTable, balance, sleep } from "../tools/helpers";
import sha256 from 'sha256'
jest.setTimeout(99999999);


test('Claim experied action', async () => {
  async function claim(contract, claimer, looser) {
    await cleos(`push action ${contract} claimexpired '["${claimer}", 0]' -p ${claimer}`)
    expect(await gamesTable(contract)).toEqual([])
    expect(await balance(contract)).toBe('0.0400 EOS')
    expect(await balance(claimer)).toBe('1000.9600 EOS')
    expect(await balance(looser)).toBe('999.0000 EOS')
  }
  async function startGame() {
    const [contract, player1, player2] = await Promise.all([deployContract(), getPlayer(), getPlayer()])
    await cleos(`push action eosio.token transfer '[ "${player1}","${contract}", "1.0000 EOS", "" ]' -p ${player1}`)
    await cleos(`push action eosio.token transfer '[ "${player2}","${contract}", "1.0000 EOS", "" ]' -p ${player2}`)
    expect(await gamesTable(contract)).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          player1,
          player2,
        })
      ])
    )
    return [contract, player1, player2]
  }
  // run all scenarios in paralel for testing all in 2 minutes
  const scenarios = [
    // player1 claim reward after joining player2
    async () => {
      const [contract, player1, player2] = await startGame()
      await sleep(121 * 1000)
      await claim(contract, player1, player2)
    },
    // player2 claim reward after joining player2
    async () => {
      const [contract, player1, player2] = await startGame()
      await sleep(121 * 1000)
      await claim(contract, player2, player1)
    },
    // no one cant claim reward after 1 minute
    async () => {
      const [contract, player1, player2] = await startGame()
      await sleep(61 * 1000)
      try {
        await claim(contract, player2, player1)
        expect(true).toBe(false)
      } catch (e) { }
      try {
        await claim(contract, player1, player2)
        expect(true).toBe(false)
      } catch (e) { }
      await sleep(61 * 1000)
      await claim(contract, player2, player1)
    },
    // player1 commit his move in one minute, then in one minute after all player1 can claim experied, but player2 cant
    async () => {
      const [contract, player1, player2] = await startGame()
      await sleep(60 * 1000)
      const committment1 = sha256('111' + 'somesecret')
      await cleos(`push action ${contract} commitmove '["${player1}", 0, "${committment1}"]' -p ${player1}`)
      await sleep(60 * 1000)
      try {
        // player2 cant claim reward
        await claim(contract, player2, player1)
        expect(true).toBe(false)
      } catch (e) {
      }
      // player1 can
      await claim(contract, player1, player2)
    },
    // player2 commit his move in one minute, then in one minute after all player1 can claim experied, but player1 cant
    async () => {
      const [contract, player1, player2] = await startGame()
      await sleep(60 * 1000)
      const committment2 = sha256('111' + 'somesecret')
      await cleos(`push action ${contract} commitmove '["${player2}", 0, "${committment2}"]' -p ${player2}`)
      await sleep(60 * 1000)
      try {
        // player1 cant claim reward
        await claim(contract, player1, player2)
        expect(true).toBe(false)
      } catch (e) {
      }
      // player2 can
      await claim(contract, player2, player1)
    },
  ]
  let count = 0;
  const int = setInterval(() => {
    count++
    process.stdout.write(count * 10 + "\n");
  }, 10 * 1000)
  await Promise.all(scenarios.map(script => script()))
  clearInterval(int)
})