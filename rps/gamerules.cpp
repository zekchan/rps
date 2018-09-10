#include <eosiolib/eosio.hpp>
#include <string>

namespace gamerules
{
const uint8_t MOVES_IN_FIGHT = 3;
const uint8_t NEED_TO_WIN = 2;
void assert_fight(std::string fight)
{
  eosio_assert(fight.length() == MOVES_IN_FIGHT, "need three moves");
  for (char &c : fight)
  {
    eosio_assert((c >= '1') && (c <= '5'), "incorect move value");
  }
}
uint8_t calcRoundWinner(char move1, char move2)
{
  /*
      1 - ROCK
      2 - PAPER
      3 - SCISORS
      4 - LIZARD
      5 - SPOCK
    */
  if (move1 == move2)
  {
    return 0;
  }
  if (move1 == '1')
  {
    return ((move2 == '3') || (move2 == '4')) ? 1 : 2;
  }
  if (move1 == '2')
  {
    return ((move2 == '1') || (move2 == '5')) ? 1 : 2;
  }
  if (move1 == '3')
  {
    return ((move2 == '2') || (move2 == '4')) ? 1 : 2;
  }
  if (move1 == '4')
  {
    return ((move2 == '2') || (move2 == '5')) ? 1 : 2;
  }
  if (move1 == '5')
  {
    return ((move2 == '1') || (move2 == '3')) ? 1 : 2;
  }
  return 0;
}
uint8_t calcWinner(std::string fight1, std::string fight2)
{
  uint8_t points1 = 0, points2 = 0;
  for (std::string::size_type i = 0; i < MOVES_IN_FIGHT; ++i)
  {
    switch (calcRoundWinner(fight1[i], fight2[i]))
    {
    case 1:
      points1++;
      break;
    case 2:
      points2++;
      break;
    }
  }
  if (points1 >= NEED_TO_WIN)
  {
    return 1;
  }
  if (points2 >= NEED_TO_WIN)
  {
    return 2;
  }
  return 0;
}
} // namespace gamerules