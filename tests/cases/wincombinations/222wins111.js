import { playCombination } from "../../tools/helpers";
jest.setTimeout(99999999);

test('222vs111', async () => {
  await playCombination('222', '111', 1)
})
