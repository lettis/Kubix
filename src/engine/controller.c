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
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "dicestates.h"

#define LEN_BUF 100

typedef struct{
	int  CMD;
	char args[ LEN_BUF ];
} Command;


/* forward declarations */
int printBoard(Board* board);
int printConfig(Config* config);



int getCmd( Command* cmd ){
	size_t nchars = LEN_BUF;
	char* buffer = malloc( nchars  * sizeof(char) );
	char* cmdStr;

	/* repeat, until there is a command that I understand */
	while( TRUE ){
		printf( "\n# " );
		getline( &buffer, &nchars, stdin );
	
		cmdStr = strtok( buffer, " " );
		sprintf( (*cmd).args, strtok( NULL, "\n"));
	
		nchars = strlen( cmdStr );
		if (cmdStr[ nchars-1 ] == '\n'){
			/* strip \n from cmdStr */
			cmdStr[ nchars-1 ] = '\0';
		}

		/* filter command string */
		if ( strcmp( cmdStr, "help" ) == 0 ){
			(*cmd).CMD = PRINT_HELP;
			break;
		}else if ( strcmp( cmdStr, "quit" ) == 0 ){
			(*cmd).CMD = EXIT;
			break;
		}else if ( strcmp( cmdStr, "board" ) == 0 ){
			(*cmd).CMD = PRINT_BOARD;
			break;
		}else if ( strcmp( cmdStr, "state" ) == 0 ){
			(*cmd).CMD = PRINT_DICE_STATE;
			break;
		}else if ( strcmp( cmdStr, "move" ) == 0 ){
			(*cmd).CMD = MOVE;
			break;
		}else if ( strcmp( cmdStr, "config" ) == 0 ){
			(*cmd).CMD = PRINT_CONFIG;
			break;
		}else{
			printf( "ERROR: unknown command '%s'\n", cmdStr );
		}
	}

	return OK;
}

int getMoveFromArgs(Move* move, char* args ){
/*TODO: not secure yet, crashes if input is wrong */
	(*move).dice 			= atoi( strtok( args, " " ) );
	(*(*move).relMove).x		= atoi( strtok( NULL, " " ) );
	(*(*move).relMove).y		= atoi( strtok( NULL, " " ) );
	(*(*move).relMove).firstX	= atoi( strtok( NULL, " " ) );
	return OK;
}

int execute( Command* cmd, Board* board, Config* config ){
	int i=0;
	int j=0;
	Move move;

	switch( (*cmd).CMD ){
		case PRINT_HELP:
			printf(	"kubix - a tactix engine -- online help\n"
				"--------------------------------------\n"
				"\n"
				"available commands:\n"
				"\thelp:\t\t\t\t\tprint this help\n"
				"\tquit:\t\t\t\t\tquit game\n"
				"\tboard:\t\t\t\t\tprint current board\n" 
				"\tstate <dice number>:\t\t\tprint state of given dice\n"
				"\tmove <dice number> <x> <y> <horiz>:\tmove a dice. x and y are integers, horiz is 1/0 (first horizontal / first vertical)\n"
				);
			break;
		case PRINT_BOARD:
			printBoard( board );
			break;
		case PRINT_CONFIG:
			printConfig( config );
			break;
		case PRINT_DICE_STATE:
			i = atoi( (*cmd).args );
			if ( 0<=i && i<=17 ){
				j = (*board).dices[i].state;
				printf( "dice %d has state %d:\n%s", i, j, statePic[j] );
			}else{
				printf( "ERROR: unknown dice %d\n", i );
			}
			break;
		case MOVE:
			/* parse arguments */
			getMoveFromArgs( &move, (*cmd).args );

			/* perform move for user */
			if ( moveIsValid( &move, board ) == TRUE ){
				moveDice( &move, board );
			}else{
				printf("ERROR: move not valid\n");
				break;
			}

			/* perform move for engine */
			i = evaluateMoves( board, &move, (*config).strategy, (*config).cpuLevel, (*config).cpuColor, 0, 101, TRUE );
			if ( i == 0 )	printf( "GAME OVER: engine gives up\n" );
			else		moveDice( &move, board );
			break;
		case EXIT:
			return EXIT;
		default:
			printf( "ERROR: unknown command\n" );
			return EXIT_UNKNOWN_COMMAND;
	}

	return OK;
}



int terminalGameController( Board* board, Config* config ){
	Command cmd;
	int errorCode = OK;

	/* initialize array with dice state pictograms */
	initStatePics();


	/* TODO: make it possible for engine to do first move (if engine plays white */


	while ( errorCode == OK ){
		/* get command from stdin */
		getCmd( &cmd );
		/* execute command */
		errorCode = execute( &cmd, board, config );
	}

	return OK;
}





/* print current situation */
int printBoard(Board* board){
	int x, y;

	for (y=8; y>=0; y--){
		if ( y==1 ) printf( " +y");
		if ( y==0 ) printf( " |__+x");
		printf("\t");
		for (x=0; x<9; x++){
			if ((*board).fields[x+9*y] == -1) printf( " . ");
			else printf( "%2d ", (*board).fields[x+9*y] );
		}
		printf("\n");
	}
	return 0;
}



int printConfig(Config* config){
	printf ( "engine level: %d\n", (*config).cpuLevel );
	if ( (*config).cpuColor == BLACK ){
		printf ( "engine color: black\n");
	}else{
		printf ( "engine color: white\n");
	}
	return 0;
}










/* TODO: throw this stuff away
int printDices(Board* board ){
	int d;
	for (d=0; d<18; d++){
		printf( "%2d:\t%2d\t(%d,%d)\t",
				d,
				(*board).dices[d].state,
				(*board).dices[d].x,
				(*board).dices[d].y
			);
		if ((*board).dices[d].color == -1)	printf("B\n");
		else 					printf("W\n");
	}
	return 0;
}

int printMove(Move* move ){
	printf( "dice\t(x,y)\tfirstX\n" );
	printf( "%2d\t", (*move).dice );
	printf( "(%d,%d)\t%d\n", (*(*move).relMove).x, (*(*move).relMove).y, (*(*move).relMove).firstX );
	return 0;
}

int getMove(Move* move ){
	int diceKey,x,y,firstX;
	printf("diceKey x y firstx\n");
	scanf("%d %d %d %d", &diceKey, &x,&y,&firstX);
	(*move).dice = diceKey;
	(*(*move).relMove).x = x;
	(*(*move).relMove).y = y;
	(*(*move).relMove).firstX = firstX;
	return 0;
}

*/
