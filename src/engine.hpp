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
#include <iostream>
#include <vector>
#include <list>
#include "global.hpp"
#include "config.hpp"

namespace KBX {

class RelativeMove {
  public:
    int dx;
    int dy;
    bool firstX;
    RelativeMove();
    RelativeMove(int dx, int dy, bool FIRST_X);
    RelativeMove invert();
    bool operator==(const RelativeMove& other);
    friend std::ostream& operator<< (std::ostream &out, const RelativeMove& move);
    friend std::istream& operator>> (std::istream &stream, RelativeMove& move);
};

class Move {
  public:
    int dieIndex;
    RelativeMove rel;
    Move();
    Move(int dieIndex, RelativeMove rel);
    bool operator==(const Move& other);
    operator bool() { return (dieIndex >= 0); }
    friend std::ostream& operator<< (std::ostream &out, const Move& move);
    friend std::istream& operator>> (std::istream &stream, Move& move);
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
    size_t getFormerState();
    PlayColor getColor();
    int x();
    int y();

    friend std::ostream& operator<< (std::ostream &out, const DieState& die);
    friend std::istream& operator>> (std::istream &stream, DieState& d);
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

    class less {
      public:
        bool operator()(const Evaluation& lhs, const Evaluation& rhs) const;
    };
};

class Game {
  public:
    Game(const Game& other);
    Game(GameConfig c);
    Game& operator=(const Game& other);
    ~Game();

    friend std::ostream& operator<<(std::ostream& out, const Game&);
    friend std::istream& operator>> (std::istream & stream, Game&);

    bool moveIsValid(Move move);
    void makeMove(Move move, bool storeMove = true);
    Move undoMove();
    Move redoMove();
    std::list< Move > possibleMoves(size_t dieId);

    void clearBoard();

    PlayColor getWinner();
    PlayColor getNext();
    PlayColor getAiColor();
    PlayColor getHumanColor();
    void reset();
    void setFinished(bool finished);
    bool finished();

    PlayMode playMode();
    void setPlayMode(PlayMode mode);

    size_t aiDepth();
    void setAiDepth(size_t aiDepth);

    size_t getNumberOfDice();
    DieState& getDie(size_t id);
    DieState& getDie(size_t x, size_t y);
    int getDieId(size_t x, size_t y);
    int getLastActiveDie();
    int getLastMovesVictim();

    void printFields();
    void printEvaluation(const Evaluation& eval);
    Strategy& getStrategy();

    Move evaluateNext();

  private:
    Evaluation _evaluateMoves(int level, float alpha, float beta, bool initialCall);
    // rating functions
    float _rateDiceRatio(PlayColor color);
    float _rate(PlayColor color);
    std::vector< std::vector<int> > _fields; // [9][9]
    std::vector< DieState > _dice; // [18]
    PlayMode _mode;
    size_t _aiDepth;
    Strategy _strategy;
    std::list< Move > _moveStack;
    std::list< Move > _moveStackPending;
    std::list< int > _deathStack;
    std::list< int > _deathStackPending;
    PlayColor _nextPlayer;
    bool _finished;
    void _setup();
};

} // end namespace KBX
#endif
