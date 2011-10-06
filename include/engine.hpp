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
// error codes / commands
#define OK                          0
#define GAME_OVER                   1
#define EXIT                        1
#define EXIT_ARGUMENT_PARSING_ERROR 2
#define EXIT_UNKNOWN_COMMAND        3
#define PRINT_HELP                  4
#define PRINT_BOARD                 5
#define PRINT_DICE_STATE            6
#define MOVE                        7
#define PRINT_CONFIG                8
// directions for die states
#define NORTH 0
#define SOUTH 1
#define EAST  2
#define WEST  3
#define VALUE 4
// define state for clear field
#define CLEAR -1
// define state out of game
#define DEAD 25
#define KING_WHITE          4
#define KING_BLACK         13
#define KING_FIELD_WHITE    4
#define KING_FIELD_BLACK   76

enum KBX_PlayMode{
      HUMAN_HUMAN
    , HUMAN_AI
    , AI_HUMAN
};

enum KBX_PlayColor{
      BLACK = -1
    , WHITE =  1
};


int sign(int x);
int isNumber( char* str );
void swap(int& a, int& b);


/// defines playing strategy of cpu
class KBX_Strategy{
    int coeffDiceRatio;
public:
};

/// configuration settings
class KBX_Config{
public:
	const KBX_PlayMode mode;
	const size_t cpuLevel;
	const KBX_Strategy strategy;
    KBX_Config(KBX_PlayMode mode, size_t cpuLevel, KBX_Strategy strategy);
};

/// defines the current state of a die (i.e. position, orientation, value, color, etc.)
class KBX_DieState{
    static const size_t _state[26][5];
	size_t _x;
	size_t _y;
	KBX_PlayColor _color;
	size_t _curState;
public:
    KBx_DieState(int x, int y, KBX_PlayColor color, size_t state);
    void move(size_t direction);
    void moveNorth();
    void moveSouth();
    void moveEast();
    void moveWest();
    void kill();
    size_t getValue();
    size_t getColor();
};

class KBX_RelativeMove{
public:
	const int x;
	const int y;
	const bool FIRST_X;
    KBX_RelativeMove(int x, int y, bool FIRST_X);
};

class KBX_Move{
public:
	const size_t dieIndex;
	const KBX_RelativeMove relMove;
    KBX_Move(size_t dieIndex, KBX_RelativeMove relMove);
};

class KBX_Game{
	int             _fields[9][9];
	KBX_DieState    _dice[18];
    KBX_Config      _config;
public:
    KBX_Game(KBX_Config config);
    int moveIsValid(KBX_Move& move);
    int makeMove(KBX_Move& move);
    float evaluateMoves(KBX_Move& bestMove, int color, float alpha, float beta, int firstCall);
};
#endif
