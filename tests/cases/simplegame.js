import eos, { deployContract } from "../tools/eos";
jest.setTimeout(99999999);
describe('Simple game Alise and Bob', () => {
  test('Alise create game with 1 EOS', async () => {
    const { contract, name } = await deployContract();
    await eos.transfer('alice', name, '1.0000 EOS', '')
    expect((await eos.getTableRows(true, name, name, 'games')).rows).toEqual([{
      id: 0,
      player1: 'alice',
      player2: name,
      bet: '1.0000 EOS',
      commitment1:
        '0000000000000000000000000000000000000000000000000000000000000000',
      commitment2:
        '0000000000000000000000000000000000000000000000000000000000000000',
      fight1: '',
      fight2: '',
      lastseen1: '1970-01-01T00:00:00',
      lastseen2: '1970-01-01T00:00:00',
      round: 1
    }]);
  })
})
