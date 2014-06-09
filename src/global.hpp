#ifndef GLOBAL__HPP
#define GLOBAL__HPP

namespace KBX {

enum Direction {
  NORTH = 1,
  SOUTH = 2,
  EAST = 3,
  WEST = 4
};

enum StateValue {
  VALUE = 0,
  DEAD = 25,
  CLEAR = -1
};

enum KingIds {
  KING_WHITE = 4,
  KING_BLACK = 13
};

enum PlayMode {
  HUMAN_AI = 0,
  AI_HUMAN = 1,
  HUMAN_HUMAN = 2,
};

const static char separator = ',';
const static char beginObj = '{';
const static char endObj = '}';
const static char beginList = '[';
const static char endList = ']';

enum PlayColor {
  BLACK = -1,
  WHITE = 1,
  NONE_OF_BOTH = 0
};

/* helper functions */
PlayColor inverse(PlayColor color);
/*                  */

/// defines playing strategy of AI
class Strategy {
  public:
    std::string name;
    double coeffDiceRatio;
    double patience;
    double randomness;
    Strategy();
    Strategy(const Strategy& other);
    void print() const;
    friend std::ostream& operator<< (std::ostream &out, const Strategy& s);
    friend std::istream& operator>> (std::istream &out, Strategy& s);
};

} // end namespace KBX

#endif
