import { playCombination } from "../../tools/helpers";
jest.setTimeout(99999999);

test('324vs123', async () => {
  await playCombination('324', '123', 2)
})
