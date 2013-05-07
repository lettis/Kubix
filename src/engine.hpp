/*  
 Kubix - 3D OpenGL implementation of the board game "Tactix"/"Duell" with AI
 Copyright (C) 2011  Florian Sittel & Carsten Burgard

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef ENGINE__HPP
#define ENGINE__HPP

#include <stddef.h>
#include <list>
#include <iostream>

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
    Strategy(float coeffDiceRatio);
    //TODO: replace read/write by stream operators <<
    bool write(std::ostream& out) const;
    bool read(std::istream& in);
};

/// configuration settings
//class Config {
//  public:
//    PlayMode mode;
//    size_t cpuLevel;
//    Strategy strategy;
//    Config(PlayMode mode, size_t cpuLevel, Strategy strategy);
//    //TODO: replace read/write by stream operators <<
//    bool write(std::ostream& out) const;
//    bool read(std::istream& in);
//};

class RelativeMove {
  public:
    int dx;
    int dy;
    bool firstX;
    RelativeMove();
    RelativeMove(int dx, int dy, bool FIRST_X);
    RelativeMove invert();
    bool write(std::ostream& out) const;
    bool read(std::istream& in);
};

class Move {
  public:
    size_t dieIndex;
    RelativeMove rel;
    Move();
    Move(size_t dieIndex, RelativeMove rel);
    bool write(std::ostream& out) const;
    bool read(std::istream& in);
};

/// defines the current state of a die (i.e. position, orientation, value, color, etc.)
class DieState {
  public:
    static const size_t nPossibleMoves[7];
    // list of possible (relative) moves for a die
    static const std::vector< std::vector< RelativeMove > > possibleMoves;
    static const std::vector< std::vector< RelativeMove > > initPossibleMoves();

    DieState();
    DieState(int x, int y, PlayColor color, size_t state);

    void moveOneStep(size_t direction);
    void kill();
    void revive();
    bool gotKilled();
    size_t getValue();
    size_t getCurrentState();
    PlayColor getColor();
    int x();
    int y();
    bool write(std::ostream& out) const;
    bool read(std::istream& in);

  private:
    static const size_t _state[26][5];
    int _x;
    int _y;
    PlayColor _color;
    size_t _formerState;
    size_t _curState;
};

class Evaluation {
  public:
    Evaluation(float rating);
    Evaluation(float rating, Move move);
    float rating;
    Move move;
};

class Game {
  public:
    Game(PlayMode mode, size_t aiDepth, Strategy strategy);

    bool moveIsValid(Move move);
    void makeMove(Move move, bool storeMove = true);
    Move undoMove();
    Move redoMove();
    std::list< Move > possibleMoves(size_t dieId);

    PlayColor getWinner();
    PlayColor getNext();
    void reset();

    DieState* getDie(size_t id);
    DieState* getDie(size_t x, size_t y);
    int getDieId(size_t x, size_t y);
    float rate(PlayColor color);
    //TODO: implement
    Move evaluateNext(int level);
    Evaluation evaluateMoves(int level, float alpha, float beta, bool initialCall);
    //TODO: replace read/write by stream operators <<
    bool write(std::ostream& out) const;
    bool read(std::istream& in);
    // rating functions
    float rateDiceRatio(PlayColor color);

  private:
    int _fields[9][9];
    DieState _dice[18];
    PlayMode _mode;
    size_t _aiDepth;
    Strategy _strategy;
    std::list< Move > _moveList;
    std::list< Move >::iterator _lastMove;
    PlayColor _nextPlayer;
    void _setup();
};

} // end namespace KBX
#endif
