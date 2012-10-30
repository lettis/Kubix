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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "engine.hpp"
#include "tools.hpp"

namespace KBX {
  /// switch color from BLACK to WHITE and vice versa
  PlayColor inverse(PlayColor color){
    switch(color){
      case BLACK:
        return WHITE;
      case WHITE:
        return BLACK;
      default:
        // do nothing
        return color;
    }
  }
  
  Strategy::Strategy(float coeffDiceRatio)
    : coeffDiceRatio(coeffDiceRatio)
  {}
  
  Config::Config(PlayMode mode, size_t cpuLevel, Strategy strategy)
    :  mode(mode)
      ,cpuLevel(cpuLevel)
      ,strategy(strategy)
  {}
  
  /// initialize a move
  RelativeMove::RelativeMove()
    :  dx(0)
      ,dy(0)
      ,FIRST_X(false)
  {}
  RelativeMove::RelativeMove(int dx, int dy, bool FIRST_X)
    :  dx(dx)
      ,dy(dy)
      ,FIRST_X(FIRST_X)
  {}
  /// invert the move (ie move back and reset to former position/orientation)
  RelativeMove RelativeMove::invert(){
      return RelativeMove( -dx, -dy, !this->FIRST_X);
  }

  Move::Move()
    : dieIndex(-1),
      rel( RelativeMove() )
  {}

  Move::Move(int dieIndex, RelativeMove rel)
    : dieIndex(dieIndex),
      rel(rel)
  {}
  
  /// initialize a die's state
  DieState::DieState()
    :  _x(-1)
      ,_y(-1)
      ,_color(NONE_OF_BOTH)
      ,_formerState(-1)
      ,_curState(-1)
  {}
  DieState::DieState(int x, int y, PlayColor color, size_t state)
    :  _x(x)
      ,_y(y)
      ,_color(color)
      ,_formerState(-1)
      ,_curState(state)
  {}
  /// save the number of possible moves per die value
  const size_t DieState::nPossibleMoves[7] = { 0, 4, 12, 20, 28, 36, 44 };
  /// save all possible moves
  const std::vector< std::vector<RelativeMove> > DieState::possibleMoves = initPossibleMoves();
  /// calculate possible moves per die value and save to list
  const std::vector< std::vector<RelativeMove> > DieState::initPossibleMoves(){
    std::vector< std::vector<RelativeMove> > possibleMoves;
    // this is just a placeholder on index 0 and won't be used
    possibleMoves.push_back( std::vector<RelativeMove>() );
    // calculate move possibilities for every die-value in every direction
    // (all theoretical possibilities, independent of board size, situation, etc)
    for (size_t val=1; val <= 6; val++){
      possibleMoves.push_back( std::vector<RelativeMove>() );
      // reference to current list
      std::vector<RelativeMove>& moveList = possibleMoves.back();
      // moves in all four directions without turning
      moveList.push_back( RelativeMove( val,    0,  true) );
      moveList.push_back( RelativeMove(-val,    0,  true) );
      moveList.push_back( RelativeMove(   0,  val, false) );
      moveList.push_back( RelativeMove(   0, -val, false) );
      // moves in all directions with turning
      for (size_t i=1; i < val; i++){
        /* +x,+y first x */
        moveList.push_back( RelativeMove( i, val-i,  true) );
        /* -x,+y first x */
        moveList.push_back( RelativeMove(-i, val-i,  true) );
        /* +x,+y first y */
        moveList.push_back( RelativeMove( i, val-i, false) );
        /* -x,+y first y */
        moveList.push_back( RelativeMove(-i, val-i, false) );
        /* +x,-y first x */
        moveList.push_back( RelativeMove( i, -(val-i), true) );
        /* -x,-y first x */
        moveList.push_back( RelativeMove(-i, -(val-i), true) );
        /* +x,-y first y */
        moveList.push_back( RelativeMove( i, -(val-i), false) );
        /* -x,-y first y */
        moveList.push_back( RelativeMove(-i, -(val-i), false) );
      }
    }
    return possibleMoves;
  }
  /// initialize die states
  /**
      the idea behind this list is to define every possible orientation a die
      can have. every entry stands for such an orientation. in the first column,
      you get the current value of the die, i.e. the number of the die, that is shown
      on top of it. the other numbers define the indices of the die states you get, if
      you roll the die one step north, south, east or west.
      so, if you roll a die over the board, you just have to traverse the list for the
      next die state index in order to always get the correct value of the die.
      the initial state is given by:
  
          1 up, 6 down, 2 south, 5 north, 3 east, 4 west,
  
      i.e.:
                     o-----o
                    /  1  /|
                   o-----o |
                   |     |3|
                   |  2  | o
                   |     |/ 
                   o-----o
  */
  const size_t DieState::_state[26][5] = {
     //indices define: value, north, south, east, west
      { 1, 4,16,12, 8 }
    , { 1, 9,14, 5,19 }
    , { 1,18, 6,11,15 }
    , { 1,13,10,17, 7 }
    , { 2,20, 0,13, 9 }
    , { 2,11,12,22, 1 }
    , { 2, 2,21,10,14 }
    , { 2,15, 8, 3,23 }
    , { 3, 7,19, 0,21 }
    , { 3,23, 1, 4,18 }
    , { 3, 3,22,16, 6 }
    , { 3,17, 5,20, 2 }
    , { 4, 5,17,21, 0 }
    , { 4,22, 3,18, 4 }
    , { 4, 1,23, 6,16 }
    , { 4,19, 7, 2,20 }
    , { 5, 0,20,14,10 }
    , { 5,12,11,23, 3 }
    , { 5,21, 2, 9,13 }
    , { 5, 8,15, 1,22 }
    , { 6,16, 4,15,11 }
    , { 6, 6,18, 8,12 }
    , { 6,10,13,19, 5 }
    , { 6,14, 9, 7,17 }
    , { 1,24,24,24,24 } // king's die
    , { 0,25,25,25,25 } // state == die got killed
  };
  /// set state to the one you get, when moving in given direction
  void DieState::move(size_t direction){
    this->_curState = this->_state[ this->_curState ][direction];
    switch( direction ){
      case NORTH:
        this->_y++;
        break;
      case SOUTH:
        this->_y--;
        break;
      case EAST:
        this->_x++;
        break;
      case WEST:
        this->_x--;
        break;
      default:
        // do nothing
        break;
    }
  }
  /// set state to DEAD (kill it!)
  void DieState::kill(){
    this->_formerState = this->_curState;
    this->_curState = DEAD;
  }
  /// set state to the one, before die got killed (revive it!)
  void DieState::revive(){
    this->_curState = this->_formerState;
  }
  /// check, if die got killed
  /**
      \returns true, if die got killed; else false
  */
  bool DieState::gotKilled(){
    return (this->_curState == DEAD);
  }
  /// return current value (1, ..., 6) of die
  size_t DieState::getValue(){
    return this->_state[ this->_curState ][VALUE];
  }
  /// return color (BLACK or WHITE) of die
  size_t DieState::getColor(){
    return this->_color;
  }
  /// return x position
  int DieState::x(){
    return this->_x;
  }
  /// return y position
  int DieState::y(){
    return this->_y;
  }
  
  /// initialize evaluation (without proper move, only rating)
  Evaluation::Evaluation(float rating)
    :  rating(rating)
      ,move(Move())
  {}
  /// initialize evaluation (with rating and move)
  Evaluation::Evaluation(float rating, Move move)
    :  rating(rating)
      ,move(move)
  {}
  
  /// initialize game
  Game::Game(Config config)
    : _config(config)
  {
    // initialize white dice
    this->_dice[ 0] = DieState( 0, 0, WHITE, 19 );
    this->_dice[ 1] = DieState( 1, 0, WHITE,  1 );
    this->_dice[ 2] = DieState( 2, 0, WHITE,  5 );
    this->_dice[ 3] = DieState( 3, 0, WHITE, 22 );
    this->_dice[ 4] = DieState( 4, 0, WHITE, 24 );
    this->_dice[ 5] = DieState( 5, 0, WHITE, 22 );
    this->_dice[ 6] = DieState( 6, 0, WHITE,  5 );
    this->_dice[ 7] = DieState( 7, 0, WHITE,  1 );
    this->_dice[ 8] = DieState( 8, 0, WHITE, 19 );
    // initialize black dice
    this->_dice[ 9] = DieState( 0, 8, BLACK, 17 );
    this->_dice[10] = DieState( 1, 8, BLACK,  3 );
    this->_dice[11] = DieState( 2, 8, BLACK,  7 );
    this->_dice[12] = DieState( 3, 8, BLACK, 23 );
    this->_dice[13] = DieState( 4, 8, BLACK, 24 );
    this->_dice[14] = DieState( 5, 8, BLACK, 23 );
    this->_dice[15] = DieState( 6, 8, BLACK,  7 );
    this->_dice[16] = DieState( 7, 8, BLACK,  3 );
    this->_dice[17] = DieState( 8, 8, BLACK, 17 );
    // initialize fields with ids of dice (or clear)
    for (size_t x=0; x <= 8; x++){
      this->_fields[x][0] = x;
    }
    for (size_t y=1; y <= 7; y++){
      for (size_t x=0; x <= 8; x++){
        this->_fields[x][y] = CLEAR;
      }
    }
    for (size_t x=0; x <= 8; x++){
      this->_fields[x][8] = x+9;
    }
  }
  /// move die over board
  void Game::makeMove(Move move){
    DieState& dieState = this->_dice[ move.dieIndex ];
    // delete die from current position on board
    this->_fields[ dieState.x() ][ dieState.y() ] = CLEAR;
    // get directions for horizontal movement (aka x coordinate)
    int directionX, directionY;
    if (move.rel.dx < 0) { directionX = WEST; }
    else                 { directionX = EAST; }
    // get directions for vertical movement (aka y coordinate)
    if (move.rel.dy < 0) { directionY = SOUTH; }
    else                 { directionY = NORTH; }
    // assume move to go first in y direction
    int stepsFirst      = abs( move.rel.dy );
    int directionFirst  = directionY;
    int stepsSec        = abs( move.rel.dx );
    int directionSec    = directionX;
    // swap values if move goes in x direction first
    if (move.rel.FIRST_X){
      swap( stepsFirst, stepsSec );
      swap( directionFirst, directionSec );
    }
    // traverse through dice states
    for (size_t i=stepsFirst; i>0; i--){
      // rotate in first direction
      dieState.move( directionFirst ); 
    }
    for (size_t i=stepsSec; i>0; i--){
      // rotate in second direction
      dieState.move( directionFirst ); 
    }
    // delete old die on this position before moving new die to it
    int keyOldDie = this->_fields[ dieState.x() ][ dieState.y() ];
    if (keyOldDie != CLEAR){
      this->_dice[ keyOldDie ].kill();
    }
    // move die to new position
    this->_fields[ dieState.x() ][ dieState.y() ] = move.dieIndex;
  }

  void Game::undoMove(){
    //TODO: implement, return move (for gui)
  }

  /// check if a given move is valid
  /**
      \returns true, if the move is valid, else false
  */
  bool Game::moveIsValid(Move move){
  	DieState dieState = this->_dice[ move.dieIndex ];
  	// check, if move is farer than die value allows
  	if (dieState.getValue() != fabs(move.rel.dx) + fabs(move.rel.dy)){
      return false;
    }
  	// check, if move goes off the board
    if (   dieState.x() + move.rel.dx < 0
        || dieState.x() + move.rel.dx > 8
        || dieState.y() + move.rel.dy < 0
        || dieState.y() + move.rel.dy > 8 ){
        return false;
    }
  	// check, if there are dice on the way, that cannot be crossed
  	if (move.rel.FIRST_X){
  		// iterate over x-values (before y-iteration)
      size_t end = fabs(move.rel.dx);
  		if ( move.rel.dy == 0 ){
        end--;
      }
      for(size_t i=1; i <= end; i++){
        if ( this->_fields[ dieState.x()+i*sgn(move.rel.dx) ][ dieState.y() ]  != CLEAR ){
          // there is a die on the way => move is not possible
          return false;
        }
      }
      // iterate over y-values
      for(size_t i=1; i < fabs(move.rel.dy); i++){
        if ( this->_fields[ dieState.x()+move.rel.dx ][ dieState.y()+i*sgn(move.rel.dy) ]  != CLEAR ){
          // there is a die on the way => move is not possible
          return false;
        }
      }
    } else { // first move in y direction
      size_t end = fabs(move.rel.dy);
      if ( move.rel.dx == 0 ){
        end--;
      }
      // iterate over y-values first
      for(size_t i=1; i <= end; i++){
        if ( this->_fields[ dieState.x() ][ dieState.y()+i*sgn(move.rel.dy) ] != CLEAR ){
          // there is a die on the way => move is not possible
          return false;
        }
      }
      // iterate over x-values (after y-iteration)
      for(size_t i=1; i < fabs(move.rel.dy); i++){
        if ( this->_fields[ dieState.x()+i*sgn(move.rel.dx) ][ dieState.y()+move.rel.dy ] != CLEAR ){
          // there is a die on the way => move is not possible
          return false;
        }
      }
    }
    // check target field
    int idTarget = this->_fields[ dieState.x()+move.rel.dx ][ dieState.y()+move.rel.dy ];
    if (idTarget != CLEAR ){
      // check if it is a dice of the enemy
      if( dieState.getColor() == this->_dice[ idTarget ].getColor() ){
        // dice have same color, cannot move to this field
        return false;
      }
    }
    // everything is ok, move possible
    return true;
  }
  /// get winner of game (black or white)
  /**
      \returns black or white; null if nobody has won yet
  */
  PlayColor Game::getWinner(){
    DieState& kingW = this->_dice[ KING_WHITE ];
    DieState& kingB = this->_dice[ KING_BLACK ];
    // check if king dice gets killed
    if ( kingW.gotKilled() )  return BLACK;
    if ( kingB.gotKilled() )  return WHITE;
    // winning by conquering opponents king field
    if ( (kingW.x()==8) && (kingW.y()==4) ) return WHITE;
    if ( (kingB.x()==0) && (kingB.y()==4) ) return BLACK;
    // nobody has won yet
    return NONE_OF_BOTH;
  }
  /// get die state of die with given id
  DieState* Game::getDie(size_t id){
    if ( id < 18 ){
      return &this->_dice[ id ];
    } else {
      throw "index error: given index does not define a valid die";
    }
  }
  /// get die state of die with given coordinates
  int Game::getDieId(size_t x, size_t y){
    if ( x < 9 && y < 9 ){
      return this->_fields[x][y];
    } else {
      throw "index error: given coordinates do not define a valid field";
    }
  }
  /// get die state of die with given coordinates
  DieState* Game::getDie(size_t x, size_t y){
    if ( x < 9 && y < 9 ){
      int id = this->_fields[x][y];
      return &this->_dice[ id ];
    } else {
      throw "index error: given coordinates do not define a valid field";
    }
  }
  /// rate the current game for a specified playcolor (black or white)
  /**
      Rate the board for a specified side with all rating functions and
      value the single ratings based on the defined strategy.
  */
  float Game::rate( PlayColor color ){
    PlayColor winner;
    // check, if game is over
    winner = this->getWinner();
    // best/worst case if winning condition reached
    if ( winner ){
      if ( winner == color )  return 100.0;
      else                    return   0.0;
    }
    float rating=0;
    rating += this->_config.strategy.coeffDiceRatio * this->rateDiceRatio( color );
    return rating;
  }
  /// evaluate best possible move up to a certain level
  /**
      TODO: document return value
      this is done recursively by a form of the NegaMax algorithm with alpha-beta pruning
  */
  Evaluation Game::evaluateMoves(int level, PlayColor color, float alpha, float beta, bool initialCall){
    // container for best move candidates
    std::vector<Evaluation> bestCandidates;
    // limit indices to significant color
    size_t from, to;
    if ( color == WHITE ){
      from =  0;
      to   =  8;
    } else {
      from =  9;
      to   = 17;
    }
    // iterate over all dice of a color
    for (size_t d=from; d <= to; d++){
      // get value of current die 
      size_t value = this->_dice[d].getValue();
      // iterate over max number of moves for given dice value (stored in state-array) 
      for (size_t i=0; i < DieState::nPossibleMoves[ value ]; i++){
        // check if this specific move is valid 
        RelativeMove move = DieState::possibleMoves[value][i];
        
        if ( this->moveIsValid( Move(d, move) ) ){
          // store all data to undo move later 
          RelativeMove moveBack = move.invert();
          // kill the die lying on the target field
          int idDieOnTarget = this->_fields[ this->_dice[d].x() + move.dx ] [ this->_dice[d].y() + move.dy ];
          if ( idDieOnTarget != CLEAR){
            this->_dice[ idDieOnTarget ].kill();
          }
          // perform move 
          this->makeMove( Move(d, move) );
          // get rating, either directly
          // or by recursive call
          float rating;
          if ( level == 1 ){
            rating = this->rate( color );
          } else {
            // recursive call for next step (negative weighting, since it is opponent's turn)
            rating = -this->evaluateMoves( level-1, inverse(color), -beta, -alpha, false ).rating;
          }
          // undo move 
          this->makeMove( Move(d, moveBack) );
          // revive killed die on target field
          if( idDieOnTarget != CLEAR ){
            this->_dice[ idDieOnTarget ].revive();
          }
          // alpha-beta pruning 
          if ( rating >= beta ){
            return Evaluation(beta);
          }
          if ( rating >= alpha){
            alpha = rating;
            if ( initialCall == true ){
              if ( rating > alpha ){
                // clear the candidate list, if rating better than before
                bestCandidates.clear();
              }
              // add good move to candidate list
              bestCandidates.push_back( Evaluation(rating, Move(d, move)) );
            }
          }
        }
      }
    }
    if ( initialCall == true ){
      // select move randomly from the list
      std::random_shuffle( bestCandidates.begin(), bestCandidates.end() );
      return bestCandidates[0];
    }
    return Evaluation(alpha);
  }
  
  /// rate dice ratio
  /**
      rate according to the ratio of the number of dice on the board
      values:
          100%:	only own dices on board
            0%:   only opponent's dices on board
  */
  float Game::rateDiceRatio(PlayColor color){
    // default rating: 50% 
    float rating = 50;
    for ( size_t i=0; i <= 17; i++ ){
      if ( this->_dice[i].gotKilled() ){
        // i from 0  to 8 resembles white dice, from 9 to 17 black dice
        if (  (i<=8 && color==WHITE) || (i>=9 && color==BLACK)  ){
          // subtract 5.5% for lost dice 
          rating -= 5.5;
        } else {
          // add 5.5% for opponents loss of dice 
          rating += 5.5;
        }
      }
    }
    return rating;
  }
} // end namespace KBX




// TODO: all lines below seem not to be needed anymore: delete

//int filterMoves(int *returnBuffer, int diceKey, Board* board ){
//	int i,v=0;
//	/* value of given dice */
//	int value = state[ (*board).dices[diceKey].state ][VALUE];
//	Move move;
//	move.dice = diceKey;
//
//	/* iterate over max number of moves for given dice value (stored in state-array) */
//	for (i=0; i<numberOfPosMoves[ value ]; i++){
//		/* check if this specific move is valid */
//		move.relMove = &possibleMoves[value][i];
//		if( moveIsValid( &move, board ) == 1 ){
//			/* ok, move is valid */
//			returnBuffer[v] = i;
//			v++;
//		}
//	}
//					/* stop signal for list iteration */
//	if ( v < numberOfPosMoves[value] )	returnBuffer[v] = -1;
//
//	return 0;
//}

//* TODO: keep defaults in config file */
//* set defaults */
//(*config).mode = HUMAN_VS_CPU;
//(*config).cpuColor = BLACK;
//(*config).cpuLevel = 4;
//(*config).strategy.coeffDiceRatio = 1;
//int main(int argc, char* argv[]){
//	int i;
//	int errorCode;
//	Config config;
//
//	/* the tactix-board
//	   Board consists of two elements:
//	  	1. fields: resemble the single fields of the board, is of type int [81]
//	  	2. dices:  list of dices */
//	Board board;
//
//	/* parse command line arguments */
//	errorCode = parseArgs( argc, argv, &config );
//
//	if ( errorCode != OK ){
//		if ( errorCode == EXIT ){
//			return OK;
//		}else{
//			return errorCode;
//		}
//	}
//
//	/* 'possibleMoves' describes all theoretically possible moves of a dice
//	   dependent on its value. the description is not dependent on the position
//	   of the dice or the current game situation, this has to be filtered in another
//	   function. advantages are, that one has not to recalculate every move, but
//	   just to iterate over the possible moves of the dice and check these. */
//	initializeMoveArray();
//
//	/* the states array defines the current state of a dice.
//	   its attributes are the value of the dice and keys of other entries
//	   in the states array, dependent on the direction the dice is rolled.
//	   that way wo do not have to calculate the state of the dice every move,
//	   but rather follow a list of keys */
//	initializeDiceStates();
//
//
//	/* select game mode */
//	switch(config.mode){
//		case HUMAN_VS_CPU:
//			errorCode = terminalGameController(&board, &config);
//			break;
//		default:
//			printf( \n"ERROR: unknown game mode\n" );
//			return EXIT_UNKNOWN_COMMAND;
//	}
//
//	return errorCode;
//}
