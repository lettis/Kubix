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

#include "engine.h"



/* checks if game is over. return value: winner */
int gameOver( Board* board ){
	Dice kingW = (*board).dices[ KING_WHITE ];
	Dice kingB = (*board).dices[ KING_BLACK ];

	/* check if king dice gets killed */
	if ( kingW.state == DEAD )  return BLACK;
	if ( kingB.state == DEAD )  return WHITE;

	/* winning by conquering opponents king field */
	if ( kingW.x + 9*kingW.y == KING_FIELD_BLACK ) return WHITE;
	if ( kingB.x + 9*kingB.y == KING_FIELD_WHITE ) return BLACK;
	
	/* nobody has won yet */
	return NONE;
}



int rateDiceRatio( Board* board, int color ){
	int i;
	/* default rating: 50% */
	float rating = 50;
	for ( i=0; i<19; i++ ){
		if ( (*board).dices[i].state == DEAD ){
			/* i from 0  to 8 resembles white dices, from 9 to 18 black dices */
			if (  (i<9 && color==WHITE) || (i>8 && color==BLACK)  )
				/* subtract 5.5% for lost dice */
				rating -= 5.5;
			else
				/* add 5.5% for opponents loss of dice */
				rating += 5.5;
		}
	}
	return rating;
}


/* rate board in context of given color and return rating */
float rateBoard( Board* board, int color, Strategy strategy ){
	int winner;
	float rating=0;

	winner = gameOver( board );

	if ( winner != NONE ){
		if ( winner == color )	return 100;
		else			return 0;
	}

	rating += strategy.coeffDiceRatio *  rateDiceRatio( board, color );
	return rating;
}

