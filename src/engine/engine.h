
#define __engine_h


/* error codes / commands */
#define OK				0
#define GAME_OVER			1
#define EXIT				1
#define EXIT_ARGUMENT_PARSING_ERROR	2
#define EXIT_UNKNOWN_COMMAND		3
#define PRINT_HELP			4
#define PRINT_BOARD			5
#define PRINT_DICE_STATE		6
#define MOVE				7
#define PRINT_CONFIG			8

/* color handling */
#define NONE   0
#define WHITE  1
#define BLACK -1
#define SWITCH_COLOR(a)  -1*a
#define KING_WHITE  4
#define KING_BLACK 13
#define KING_FIELD_WHITE  4
#define KING_FIELD_BLACK 76

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
typedef struct {
	int coeffDiceRatio;
} Strategy;


/* configuration */
typedef struct {
	int mode;
	int cpuColor;
	int cpuLevel;
	Strategy strategy;
} Config;



typedef struct {
	int state;
	int x;
	int y;
	int color;
} Dice;

typedef struct {
	int	fields[81];
	Dice	dices[18];
} Board;

typedef struct {
	int x;
	int y;
	int firstX;
} RelMove;

typedef struct {
	int		dice;
	RelMove*	relMove;
} Move;



/* forward declarations */
int moveDice(Move* move, Board* board );
int moveIsValid(Move* move, Board* board );
float evaluateMoves(Board* board, Move* bestMove, Strategy strategy, int level, int color, float alpha, float beta, int firstCall);



/* includes of sub-modules */
#ifndef __rating_h
 #include "rating.h"
#endif

#ifndef __terminal_h
 #include "controller.h"
#endif

