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

/* error codes / commands */
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

/* color handling */
#define NONE                0
#define WHITE               1
#define BLACK              -1
#define SWITCH_COLOR(a)  -1*a
#define KING_WHITE          4
#define KING_BLACK         13
#define KING_FIELD_WHITE    4
#define KING_FIELD_BLACK   76

/* directions */
#define NORTH 0
#define SOUTH 1
#define EAST  2
#define WEST  3
#define VALUE 4

#define TRUE  1
#define FALSE 0

/* define state out of game */
#define DEAD 25

/* game modes */
#define HUMAN_VS_CPU     1
#define NETWORK_VS_CPU   2
#define NETWORK_VS_HUMAN 3


/* playing strategy of cpu */
class KBX_Strategy{
	int coeffDiceRatio;
public:
};


/* configuration */
class KBX_Config{
	int mode;
	int cpuColor;
	int cpuLevel;
	Strategy strategy;
public:
};


class KBX_DieState{
	int state;
	int x;
	int y;
	int color;
public:
};

class KBX_RelativeMove{
	int x;
	int y;
	int FIRST_X;
public:
    KBX_RelativeMove(int x, int y, bool FIRST_X);
};

class KBX_Move{
	int                 die;
	KBX_RelativeMove*   relMove;
public:
};

class KBX_BoardState{
	int             fields[81];
	KBX_DieState    dice[18];
public:
    int moveIsValid(KBX_Move& move );
    int makeMove(KBX_Move& move);
    float evaluateMoves(KBX_Move& bestMove, KBX_Strategy strategy, int level, int color, float alpha, float beta, int firstCall);
};
#endif
