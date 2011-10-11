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

enum KBX_PlayMode{
      HUMAN_HUMAN
    , HUMAN_AI
    , AI_HUMAN
};

enum KBX_PlayColor{
      BLACK = -1
    , WHITE =  1
    , NONE_OF_BOTH = 0
};

/* helper functions */
int sign(int x);
int isNumber( char* str );
void swap(int& a, int& b);
KBX_PlayColor inverse(KBX_PlayColor color);
/*                  */

/// defines playing strategy of cpu
class KBX_Strategy{
public:
    const int coeffDiceRatio;
    KBX_Strategy( float coeffDiceRatio );
};

/// configuration settings
class KBX_Config{
public:
	const KBX_PlayMode mode;
	const size_t cpuLevel;
	const KBX_Strategy strategy;
    KBX_Config(KBX_PlayMode mode, size_t cpuLevel, KBX_Strategy strategy);
};

class KBX_Move{
public:
    int dx;
    int dy;
    bool FIRST_X;
    KBX_Move();
    KBX_Move(int dx, int dy, bool FIRST_X);
    KBX_Move invert();
};

/// defines the current state of a die (i.e. position, orientation, value, color, etc.)
class KBX_DieState{
    static const size_t _state[26][5];
	int _x;
	int _y;
	KBX_PlayColor _color;
    size_t _formerState;
	size_t _curState;
public:
    static const size_t nPossibleMoves[7];
    // list of possible (relative) moves for a die
    static const std::vector< std::vector<KBX_Move> > possibleMoves;
    static const std::vector< std::vector<KBX_Move> > initPossibleMoves();

    KBX_DieState();
    KBX_DieState(int x, int y, KBX_PlayColor color, size_t state);

    void move(size_t direction);
    void kill();
    void revive();
    bool gotKilled();
    size_t getValue();
    size_t getColor();
    int x();
    int y();
};

class KBX_Evaluation{
public:
    float rating;
    int dieIndex;
    KBX_Move move;
    KBX_Evaluation(float rating);
    KBX_Evaluation(float rating, int dieIndex, KBX_Move move);
};

class KBX_Game{
	int          _fields[9][9];
	KBX_DieState _dice[18];
    KBX_Config   _config;
public:
    KBX_Game(KBX_Config config);
    bool            moveIsValid(size_t dieIndex, KBX_Move& move);
    void            makeMove(size_t dieIndex, KBX_Move& move);
    KBX_PlayColor   getWinner();
    float           rate(KBX_PlayColor color);
    KBX_Evaluation  evaluateMoves(int level, KBX_PlayColor color, float alpha, float beta, bool initialCall);
    // rating functions
    float           rateDiceRatio(KBX_PlayColor color);
};
#endif
