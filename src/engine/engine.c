#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "engine.h"


/* ----- tools ------------------ */
int sign(int x){
/* returns */
/*	 1, if sign of x is positive */
/*	-1, if sign of x is negative */
/*	 0, if x is zero */
	if	( x==0 )	return  0;
	else if ( x >0 )	return  1;
	else			return -1;
}

int isNumber( char* str ){
	if ( strspn( str, "1234567890 " ) == strlen( str ) )	return TRUE;
	else							return FALSE;
}
/* ----- end tools -------------- */


/* list of possible dice-states */
/* TODO: write more about the idea behind the dice states */
/* TODO: define dice states in header file */
int state[26][5];

/* initialize dice state list */
int initState(int* state, int val, int n, int s, int e, int w){
	state [NORTH]	= n;
	state [SOUTH]	= s;
	state [EAST]	= e;
	state [WEST]	= w;
	state [VALUE]	= val;
	return 0;
}

/* initialize dice states */
void initializeDiceStates(){
	/* initial state:  1 up, 6 down, 2 south, 5 north, 3 east, 4 west */
	initState(state [0], 1, 4,16,12, 8 );
	initState(state [1], 1, 9,14, 5,19 );
	initState(state [2], 1,18, 6,11,15 );
	initState(state [3], 1,13,10,17, 7 );
	initState(state [4], 2,20, 0,13, 9 );
	initState(state [5], 2,11,12,22, 1 );
	initState(state [6], 2, 2,21,10,14 );
	initState(state [7], 2,15, 8, 3,23 );
	initState(state [8], 3, 7,19, 0,21 );
	initState(state [9], 3,23, 1, 4,18 );
	initState(state[10], 3, 3,22,16, 6 );
	initState(state[11], 3,17, 5,20, 2 );
	initState(state[12], 4, 5,17,21, 0 );
	initState(state[13], 4,22, 3,18, 4 );
	initState(state[14], 4, 1,23, 6,16 );
	initState(state[15], 4,19, 7, 2,20 );
	initState(state[16], 5, 0,20,14,10 );
	initState(state[17], 5,12,11,23, 3 );
	initState(state[18], 5,21, 2, 9,13 );
	initState(state[19], 5, 8,15, 1,22 );
	initState(state[20], 6,16, 4,15,11 );
	initState(state[21], 6, 6,18, 8,12 );
	initState(state[22], 6,10,13,19, 5 );
	initState(state[23], 6,14, 9, 7,17 );
	initState(state[24], 1,24,24,24,24 ); /* king's die */
	initState(state[25], 0,25,25,25,25 ); /* DESTROY!!! */
}





/* list of possible (relative) moves for a die */
RelMove* possibleMoves[7];
int numberOfPosMoves[7];

/* initialize given relative move */
int initMove (RelMove* move, int x, int y, int firstX){
	(*move).x = x;
	(*move).y = y;
	(*move).firstX = firstX;
	return 0;
}

/* initialize array "possibleMoves" */
void initializeMoveArray(){
	int val,i;

	/* save the number of possible moves per dice value */
	numberOfPosMoves[0] =  0;
	numberOfPosMoves[1] =  4;
	numberOfPosMoves[2] = 12;
	numberOfPosMoves[3] = 20;
	numberOfPosMoves[4] = 28;
	numberOfPosMoves[5] = 36;
	numberOfPosMoves[6] = 44;

	/* allocate memory for possible-moves array */
	possibleMoves[0] = NULL;
	possibleMoves[1] = (RelMove*) malloc(  4 * sizeof(RelMove));
	possibleMoves[2] = (RelMove*) malloc( 12 * sizeof(RelMove));
	possibleMoves[3] = (RelMove*) malloc( 20 * sizeof(RelMove));
	possibleMoves[4] = (RelMove*) malloc( 28 * sizeof(RelMove));
	possibleMoves[5] = (RelMove*) malloc( 36 * sizeof(RelMove));
	possibleMoves[6] = (RelMove*) malloc( 44 * sizeof(RelMove));

	/* calculate move possibilities for every dice-value in every direction
	  (all theoretical possibilities, independent of board size, situation, etc) */
	for ( val=1; val<7; val++ ){
		initMove( &possibleMoves[val][0],  val,    0, 1 );
		initMove( &possibleMoves[val][1], -val,    0, 1 );
		initMove( &possibleMoves[val][2],    0,  val, 0 );
		initMove( &possibleMoves[val][3],    0, -val, 0 );

		/* TODO: write more about the funny indices */
		for ( i=1; i<val; i++ ){
										/* +x,+y first x */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 4 ],   i,  val-i, TRUE  );
										/* -x,+y first x */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 5 ],  -i,  val-i, TRUE  );
										/* +x,+y first y */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 6 ],   i,  val-i, FALSE );
										/* -x,+y first y */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 7 ],  -i,  val-i, FALSE );

										/* +x,-y first x */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 8 ],   i, -(val-i), TRUE  );
										/* -x,-y first x */
			initMove( &possibleMoves [val]   [ (i-1)*8 + 9 ],  -i, -(val-i), TRUE  );
										/* +x,-y first y */
			initMove( &possibleMoves [val]   [ (i-1)*8 +10 ],   i, -(val-i), FALSE );
										/* -x,-y first y */
			initMove( &possibleMoves [val]   [ (i-1)*8 +11 ],  -i, -(val-i), FALSE );
		}
	}
}






/* move die over board */
int moveDice(Move* move, Board* board ){
	int i=0;

	int xVal = (*(*move).relMove).x, dirX;
	int yVal = (*(*move).relMove).y, dirY;
	int first, sec, dirFirst, dirSec;
	int keyOldDice=-1;

	/* get current state of this die */
	int stateNow = (*board).dices[ (*move).dice ].state;

	if ((*(*move).relMove).x < 0){
		xVal = (-1)*xVal;
		dirX = WEST;
	} else{
		dirX = EAST;
	}

	if ((*(*move).relMove).y < 0){
		yVal = (-1)*yVal;
		dirY = SOUTH;
	} else{
		dirY = NORTH;
	}

	if ((*(*move).relMove).firstX){
		first=xVal; dirFirst=dirX;
		sec  =yVal; dirSec  =dirY;
	} else{
		first = yVal; dirFirst=dirY;
		sec   = xVal; dirSec  =dirX;
	}

	/* traverse through dice states */
	for ( i=first; i>0; i--){
		/* rotate in first direction */
		stateNow = state[ stateNow ][ dirFirst ];
	}

	for ( i=sec; i>0; i--){
		/* rotate in second direction */
		stateNow = state[ stateNow ][ dirSec ];
	}

	/* write new current state to die */
	(*board).dices[ (*move).dice ].state = stateNow;

	/* delete dice from current position on board */
	(*board).fields[ (*board).dices[ (*move).dice ].x + 9*(*board).dices[ (*move).dice ].y ] = -1;

	/* write new position to die */
	(*board).dices[ (*move).dice ].x += (*(*move).relMove).x;
	(*board).dices[ (*move).dice ].y += (*(*move).relMove).y;

	/* set new position on board */
	if ((*board).fields[ (*board).dices[ (*move).dice ].x + 9*(*board).dices[ (*move).dice ].y ]  != -1 ){
		/* delete dice on this position before moving new dice to this position
		   ( by setting its coordinates to -1 */
		keyOldDice = (*board).fields[ (*board).dices[ (*move).dice ].x + 9*(*board).dices[ (*move).dice ].y ];
		(*board).dices[ keyOldDice ].state = DEAD;
	}
	/* move the new dice to given position */
	(*board).fields[ (*board).dices[ (*move).dice ].x + 9*(*board).dices[ (*move).dice ].y ] = (*move).dice;

	return 0;
}


int moveIsValid(Move* move, Board* board ){
	int i,end;
	/* *d is a pointer to the dice to be moved */
	Dice* d = &(*board).dices[(*move).dice];

	/* check, if move is farer than dice value would suggest */
	if(  state[ (*d).state ][VALUE] != fabs( (*(*move).relMove).x ) + fabs( (*(*move).relMove).y )  ) return FALSE;

	/* check, if move goes off the board */
	if (  (*d).x + (*(*move).relMove).x < 0  ||  (*d).x + (*(*move).relMove).x > 8  ) return FALSE;
	if (  (*d).y + (*(*move).relMove).y < 0  ||  (*d).y + (*(*move).relMove).y > 8  ) return FALSE;

	/* check, if there are dices on the way, that cannot be crossed */
	if ((*(*move).relMove).firstX == TRUE){
		/* iterate over x-values (before y-iteration) */

		if ( (*(*move).relMove).y == 0 )	end = fabs( (*(*move).relMove).x  ) -1;
		else					end = fabs( (*(*move).relMove).x  );

		for(i=1; i<=end; i++)
			if ( (*board).fields[    (*d).x + i*sign( (*(*move).relMove).x )     + 9*(*d).y     ]  != -1 ){
				/* there is a dice on the way, move not possible */
				return FALSE;
			}

		/* iterate over y-values */
		for(i=1; i<fabs( (*(*move).relMove).y ); i++)
			if ( (*board).fields[ (*d).x+(*(*move).relMove).x + 9*((*d).y+i*sign((*(*move).relMove).y) )     ]  != -1 ){
				/* there is a dice on the way, move not possible */
				return FALSE;
			}
	} else {

		if ( (*(*move).relMove).x == 0 )	end = fabs( (*(*move).relMove).y  ) -1;
		else					end = fabs( (*(*move).relMove).y  );

		/* iterate over y-values first */
		for(i=1; i<=end; i++)
			if ( (*board).fields[ (*d).x + 9*(  (*d).y+i*sign( (*(*move).relMove).y )  )     ]  != -1 ){
				/* there is a dice on the way, move not possible */
				return FALSE;
			}

		/* iterate over x-values (after y-iteration) */
		for(i=1; i<fabs( (*(*move).relMove).x ); i++)
			if ( (*board).fields[    (*d).x + i*sign( (*(*move).relMove).x ) + 9*((*d).y+(*(*move).relMove).y)     ]  != -1 ){
				/* there is a dice on the way, move not possible */
				return FALSE;
			}
	}

	/* check target field */
	if ( (*board).fields[ (*d).x+(*(*move).relMove).x + 9*((*d).y+(*(*move).relMove).y) ]  != -1 ){
		/* check if it is a dice of the enemy
		   color of dice to be moved			 color of dice on target field */
		if( (*d).color == (*board).dices[ (*board).fields[ (*d).x+(*(*move).relMove).x + 9*((*d).y+(*(*move).relMove).y) ] ].color ){
			/* dices have same color, cannot move to this field */
			return FALSE;
		}
	}

	/* everything is ok, move possible */
	return TRUE;
}



int filterMoves(int *returnBuffer, int diceKey, Board* board ){
	int i,v=0;
	/* value of given dice */
	int value = state[ (*board).dices[diceKey].state ][VALUE];
	Move move;
	move.dice = diceKey;

	/* iterate over max number of moves for given dice value (stored in state-array) */
	for (i=0; i<numberOfPosMoves[ value ]; i++){
		/* check if this specific move is valid */
		move.relMove = &possibleMoves[value][i];
		if( moveIsValid( &move, board ) == 1 ){
			/* ok, move is valid */
			returnBuffer[v] = i;
			v++;
		}
	}
					/* stop signal for list iteration */
	if ( v < numberOfPosMoves[value] )	returnBuffer[v] = -1;

	return 0;
}


/* evaluate best possible move up to a certain level
   this is done recursively by a form of the NegaMax algorithm with alpha-beta pruning */
float evaluateMoves(Board* board, Move* bestMove, Strategy strategy, int level, int color, float alpha, float beta, int firstCall){
	int i,d,from,to;
	int value;
	int origX, origY;
	int origStateMovedDice;
	/* containers for dice-state and dice-key of target dice (if there is one) */
	int origStateTargetDice=25, diceOnTargetField;
   	float ratingValue;

	Move move, bestMoveLocal;

	if ( level == 0 ){
		return rateBoard( board, color, strategy );
	}

	if ( color == WHITE )	{ from=0; to=9; }
	else			{ from=9; to=18; }

	/* iterate over all dices of a color */
	for (d=from; d<to; d++){
		/* get value of current dice */
		value = state[ (*board).dices[d].state ][VALUE];
		/* iterate over max number of moves for given dice value (stored in state-array) */
		for (i=0; i<numberOfPosMoves[ value ]; i++){
			/* check if this specific move is valid */
			move.relMove = &possibleMoves[value][i];
			move.dice = d;

			if ( moveIsValid( &move, board ) == TRUE ){
				/* store all data to undo move later */
				origX				= (*board).dices[ move.dice ].x;
				origY				= (*board).dices[ move.dice ].y;
				origStateMovedDice		= (*board).dices[ move.dice ].state;
				diceOnTargetField		= (*board).fields[  origX + (*move.relMove).x  +  9*( origY+(*move.relMove).y )  ];
				if ( diceOnTargetField != -1)	origStateTargetDice = (*board).dices[ diceOnTargetField ].state;

				/* perform move */
				moveDice( &move, board );

				/* recursive call for next step */
				ratingValue = -evaluateMoves( board, bestMove, strategy, level-1, SWITCH_COLOR(color), -beta, -alpha, FALSE );

				/* undo move */
				(*board).dices[ move.dice ].x								= origX;
				(*board).dices[ move.dice ].y								= origY;
				(*board).dices[ move.dice ].state							= origStateMovedDice;
				(*board).fields[ origX+9*origY ]							= move.dice;
				(*board).fields[  origX + (*move.relMove).x  +  9*( origY+(*move.relMove).y )  ]	= diceOnTargetField;
				if( diceOnTargetField != -1)		(*board).dices[ diceOnTargetField ].state	= origStateTargetDice;

				/* alpha-beta pruning */
				if ( ratingValue >= beta ){
					return beta;
				}

				if ( ratingValue > alpha){
					alpha = ratingValue;
					/* TODO: only overwrite if move is better than bestMoveLocal,
						 if moves are equal, select randomly */
					if ( firstCall == TRUE ){
      						bestMoveLocal = move;
					}
				}
			}
		}
	}

	if ( firstCall == TRUE )	*bestMove = bestMoveLocal;
	return alpha;
}


/* initialize single dice */
int initDice( Dice* dice, int state, int x, int y, int color ){
	(*dice).state = state;
	(*dice).x = x;
	(*dice).y = y;
	(*dice).color = color;
	return 0;
}

/* parse command line arguments */
int parseArgs(int argc, char* argv[], Config* config ){
	int i=1;
	char* help=	"kubix - a tactix engine\n"
			"-----------------------\n"
			"\t-h:\tprint this help\n"
			"\t-net:\tstart network server (NOT YET IMPLEMENTED)\n"
			"\t-level:\tset cpu depth level (default: 4)\n";

	/* TODO: keep defaults in config file */
	/* set defaults */
	(*config).mode = HUMAN_VS_CPU;
	(*config).cpuColor = BLACK;
	(*config).cpuLevel = 4;

	/* default strategy */
	(*config).strategy.coeffDiceRatio = 1;


	/* parse arguments */
	while ( i < argc ){
		if ( strcmp( argv[i], "-h") == 0 ){
			printf("%s\n", help);
			return EXIT;
/* TODO: not yet implemented
		}else if( strcmp( argv[i], "-net") == 0 ){
			(*config).mode = NETWORK_VS_CPU;
*/
		}else if ( strcmp( argv[i], "-level" ) == 0 ){
			i++;
			if ( i > argc ){
				printf( "ERROR: not enough arguments\n" );
				return EXIT_ARGUMENT_PARSING_ERROR;
			}
			if ( isNumber( argv[i] ) == TRUE ){
				(*config).cpuLevel = atoi( argv[i] );
			}else{
				printf( "ERROR: -level argument '%s' does not make sense\n", argv[i] );
				return EXIT_ARGUMENT_PARSING_ERROR;
			}
		}else{
			printf("ERROR: unknown argument '%s'\n", argv[i]);
			return EXIT_ARGUMENT_PARSING_ERROR;
		}
		i++;
	}

	return OK;
}


int main(int argc, char* argv[]){
	int i;
	int errorCode;
	Config config;

	/* the tactix-board
	   Board consists of two elements:
	  	1. fields: resemble the single fields of the board, is of type int [81]
	  	2. dices:  list of dices */
	Board board;

	/* parse command line arguments */
	errorCode = parseArgs( argc, argv, &config );

	if ( errorCode != OK ){
		if ( errorCode == EXIT ){
			return OK;
		}else{
			return errorCode;
		}
	}

	/* 'possibleMoves' describes all theoretically possible moves of a dice
	   dependent on its value. the description is not dependent on the position
	   of the dice or the current game situation, this has to be filtered in another
	   function. advantages are, that one has not to recalculate every move, but
	   just to iterate over the possible moves of the dice and check these. */
	initializeMoveArray();

	/* the states array defines the current state of a dice.
	   its attributes are the value of the dice and keys of other entries
	   in the states array, dependent on the direction the dice is rolled.
	   that way wo do not have to calculate the state of the dice every move,
	   but rather follow a list of keys */
	initializeDiceStates();


	/* params of initDice:
	               diceStruct*, state, x, y, color */
	/* initielize white dices */
	initDice( &board.dices[0], 19, 0,0, WHITE );
	initDice( &board.dices[1],  1, 1,0, WHITE );
	initDice( &board.dices[2],  5, 2,0, WHITE );
	initDice( &board.dices[3], 22, 3,0, WHITE );
	initDice( &board.dices[4], 24, 4,0, WHITE );
	initDice( &board.dices[5], 22, 5,0, WHITE );
	initDice( &board.dices[6],  5, 6,0, WHITE );
	initDice( &board.dices[7],  1, 7,0, WHITE );
	initDice( &board.dices[8], 19, 8,0, WHITE );

	/* initialize black dices */
	initDice( &board.dices[ 9], 17, 0,8, BLACK );
	initDice( &board.dices[10],  3, 1,8, BLACK );
	initDice( &board.dices[11],  7, 2,8, BLACK );
	initDice( &board.dices[12], 23, 3,8, BLACK );
	initDice( &board.dices[13], 24, 4,8, BLACK );
	initDice( &board.dices[14], 23, 5,8, BLACK );
	initDice( &board.dices[15],  7, 6,8, BLACK );
	initDice( &board.dices[16],  3, 7,8, BLACK );
	initDice( &board.dices[17], 17, 8,8, BLACK );

	/* board index: x+9*y */
	for ( i=0; i<9; i++ ){
		/* init white dices */
		board.fields[i] = i;
		/* init black dices */
		board.fields[i+72] = i+9;
	}

	/* set blank fields */
	for ( i=9; i<72; i++ )
		board.fields[i] = -1;


	/* select game mode */
	switch(config.mode){
		case HUMAN_VS_CPU:
			errorCode = terminalGameController(&board, &config);
			break;
		default:
			printf( "ERROR: unknown game mode\n" );
			return EXIT_UNKNOWN_COMMAND;
	}

	return errorCode;
}
