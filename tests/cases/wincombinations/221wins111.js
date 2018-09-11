import { playCombination } from "../../tools/helpers";
jest.setTimeout(99999999);

test('player1 wins player2 221vs111', async () => {
  await playCombination('221', '111', 1)
})
