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
  HUMAN_HUMAN,
  HUMAN_AI,
  AI_HUMAN
};

const static char separator = '\n';

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
    int coeffDiceRatio;
    Strategy();
    //TODO: replace read/write by stream operators <<
    bool write(std::ostream& out) const;
    bool read(std::istream& in);
};

} // end namespace KBX

#endif
