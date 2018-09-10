import eos, { deployContract } from "../tools/eos";
jest.setTimeout(99999999);
describe('Simple game Alise and Bob', () => {
  test('Alise wins bob', async () => {
    const looper = () => deployContract().then((name) => {
      console.log(name);
      return looper();
    })
    await looper();
  })
})