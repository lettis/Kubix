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

#define __rating_h

/* rate the board for a specified side with all rating functions and
   value the single ratings based on strategy */
float rateBoard(Board* board, int color, Strategy strategy);


/*
//// rating functions ////

// numberOfDices: //
//
// represents the ratio of the number of dices on the board
// 	values:
//		100%:	only own dices on board
//		  0%:   only opponent's dices on board
*/
int rateDiceRatio(Board* board, int color);


