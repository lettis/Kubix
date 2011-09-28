
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


