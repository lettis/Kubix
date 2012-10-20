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

//TODO: enums!

// directions for die states
#define VALUE 0
#define NORTH 1
#define SOUTH 2
#define EAST  3
#define WEST  4
// define state for clear field (i.e., field without a die)
#define CLEAR -1
// define alias for state 'die got killed'
#define DEAD        25
// define aliases for die indices of kings
#define KING_WHITE   4
#define KING_BLACK  13

namespace KBX {
  enum PlayMode{
        HUMAN_HUMAN
      , HUMAN_AI
      , AI_HUMAN
  };
  
  enum PlayColor{
        BLACK = -1
      , WHITE =  1
      , NONE_OF_BOTH = 0
  };
  
  /* helper functions */
  int sign(int x);
  int isNumber( char* str );
  void swap(int& a, int& b);
  PlayColor inverse(PlayColor color);
  /*                  */
  
  /// defines playing strategy of cpu
  class Strategy{
  public:
      const int coeffDiceRatio;
      Strategy( float coeffDiceRatio );
  };
  
  /// configuration settings
  class Config{
  public:
  	const PlayMode mode;
  	const size_t cpuLevel;
  	const Strategy strategy;
      Config(PlayMode mode, size_t cpuLevel, Strategy strategy);
  };
  
  class Move{
  public:
      int dx;
      int dy;
      bool FIRST_X;
      Move();
      Move(int dx, int dy, bool FIRST_X);
      Move invert();
  };
  
  /// defines the current state of a die (i.e. position, orientation, value, color, etc.)
  class DieState{
      static const size_t _state[26][5];
  	int _x;
  	int _y;
  	PlayColor _color;
      size_t _formerState;
  	size_t _curState;
  public:
      static const size_t nPossibleMoves[7];
      // list of possible (relative) moves for a die
      static const std::vector< std::vector<Move> > possibleMoves;
      static const std::vector< std::vector<Move> > initPossibleMoves();
  
      DieState();
      DieState(int x, int y, PlayColor color, size_t state);
  
      void move(size_t direction);
      void kill();
      void revive();
      bool gotKilled();
      size_t getValue();
      size_t getColor();
      int x();
      int y();
  };
  
  class Evaluation{
  public:
      float rating;
      int dieIndex;
      Move move;
      Evaluation(float rating);
      Evaluation(float rating, int dieIndex, Move move);
  };
  
  class Game{
  	int          _fields[9][9];
  	DieState _dice[18];
      Config   _config;
  public:
      Game(Config config);
      bool            moveIsValid(size_t dieIndex, Move& move);
      void            makeMove(size_t dieIndex, Move& move);
      PlayColor   getWinner();
      DieState*   getDie(size_t id);
      DieState*   getDie(size_t x, size_t y);
      int             getDieId(size_t x, size_t y);
      float           rate(PlayColor color);
      Evaluation  evaluateMoves(int level, PlayColor color, float alpha, float beta, bool initialCall);
      // rating functions
      float           rateDiceRatio(PlayColor color);
  };

} // end namespace KBX
#endif
