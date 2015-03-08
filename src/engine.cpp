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
#include <queue>
#include <algorithm>
#include <sstream>

#include "engine.hpp"
#include "tools.hpp"

namespace KBX {
/// switch color from BLACK to WHITE and vice versa
PlayColor inverse(PlayColor color) {
  switch (color) {
    case BLACK:
      return WHITE;
    case WHITE:
      return BLACK;
    default:
      // do nothing
      return color;
  }
}

Strategy::Strategy()
  : name("default"),
    coeffDiceRatio(1.),
    patience(.95)
{
}

Strategy::Strategy(const Strategy& other)
  : name(other.name),
    coeffDiceRatio(other.coeffDiceRatio),
    patience(other.patience)
{
}

void Strategy::print() const {
  std::cout << "Strategy '" << this->name << "'" << std::endl;
  std::cout << "dice ratio coefficient: " << this->coeffDiceRatio << std::endl;
  std::cout << "patience: " << this->patience << std::endl;
}

/// initialize a move
RelativeMove::RelativeMove()
    : dx(0),
      dy(0),
      firstX(false) {
}
RelativeMove::RelativeMove(int dx, int dy, bool FIRST_X)
    : dx(dx),
      dy(dy),
      firstX(FIRST_X) {
}
/// invert the move (ie move back and reset to former position/orientation)
RelativeMove RelativeMove::invert() {
  return RelativeMove( -this->dx, -this->dy, !this->firstX);
}

bool RelativeMove::operator==(const RelativeMove& other) {
  if ((this->dx == other.dx) && (this->dy == other.dy) && (this->firstX == other.firstX)) {
    return true;
  } else {
    return false;
  }
}

Move::Move()
    : dieIndex( -1),
      rel(RelativeMove()) {
}

Move::Move(int dieIndex, RelativeMove rel)
    : dieIndex(dieIndex),
      rel(rel) {
}

bool Move::operator==(const Move& other) {
  if ((this->dieIndex == other.dieIndex) && (this->rel == other.rel)) {
    return true;
  } else {
    return false;
  }
}

/// initialize a die's state
DieState::DieState()
    : _x( -1),
      _y( -1),
      _color(NONE_OF_BOTH),
      _formerState( -1),
      _curState( -1) {
}
DieState::DieState(int x, int y, PlayColor color, size_t state)
    : _x(x),
      _y(y),
      _color(color),
      _formerState( -1),
      _curState(state) {
}
/// save the number of possible moves per die value
const size_t DieState::nPossibleMoves[7] = {0, 4, 12, 20, 28, 36, 44};
/// save all possible moves
const std::vector< std::vector< RelativeMove > > DieState::possibleMoves = initPossibleMoves();
/// calculate possible moves per die value and save to list
const std::vector< std::vector< RelativeMove > > DieState::initPossibleMoves() {
  std::vector< std::vector< RelativeMove > > possibleMoves;
  // this is just a placeholder on index 0 and won't be used
  possibleMoves.push_back(std::vector< RelativeMove >());
  // calculate move possibilities for every die-value in every direction
  // (all theoretical possibilities, independent of board size, situation, etc)
  for (size_t val = 1; val <= 6; val++) {
    possibleMoves.push_back(std::vector< RelativeMove >());
    // reference to current list
    std::vector< RelativeMove >& moveList = possibleMoves.back();
    // moves in all four directions without turning
    moveList.push_back(RelativeMove(val, 0, true));
    moveList.push_back(RelativeMove( -val, 0, true));
    moveList.push_back(RelativeMove(0, val, false));
    moveList.push_back(RelativeMove(0, -val, false));
    // moves in all directions with turning
    for (size_t i = 1; i < val; i++) {
      /* +x,+y first x */
      moveList.push_back(RelativeMove(i, val - i, true));
      /* -x,+y first x */
      moveList.push_back(RelativeMove( -i, val - i, true));
      /* +x,+y first y */
      moveList.push_back(RelativeMove(i, val - i, false));
      /* -x,+y first y */
      moveList.push_back(RelativeMove( -i, val - i, false));
      /* +x,-y first x */
      moveList.push_back(RelativeMove(i, -(val - i), true));
      /* -x,-y first x */
      moveList.push_back(RelativeMove( -i, -(val - i), true));
      /* +x,-y first y */
      moveList.push_back(RelativeMove(i, -(val - i), false));
      /* -x,-y first y */
      moveList.push_back(RelativeMove( -i, -(val - i), false));
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
    {1, 4, 16, 12, 8}, {1, 9, 14, 5, 19}, {1, 18, 6, 11, 15}, {1, 13, 10, 17, 7}, {2, 20, 0, 13, 9}, {2, 11, 12, 22, 1},
    {2, 2, 21, 10, 14}, {2, 15, 8, 3, 23}, {3, 7, 19, 0, 21}, {3, 23, 1, 4, 18}, {3, 3, 22, 16, 6}, {3, 17, 5, 20, 2}, {
        4, 5, 17, 21, 0}, {4, 22, 3, 18, 4}, {4, 1, 23, 6, 16}, {4, 19, 7, 2, 20}, {5, 0, 20, 14, 10},
    {5, 12, 11, 23, 3}, {5, 21, 2, 9, 13}, {5, 8, 15, 1, 22}, {6, 16, 4, 15, 11}, {6, 6, 18, 8, 12}, {6, 10, 13, 19, 5},
    {6, 14, 9, 7, 17}, {1, 24, 24, 24, 24} // king's die
    , {0, 25, 25, 25, 25} // state == die got killed
};
/// set state to the one you get, when moving in given direction
void DieState::moveOneStep(size_t direction) {
  this->_curState = this->_state[this->_curState][direction];
  switch (direction) {
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
void DieState::kill() {
  this->_formerState = this->_curState;
  this->_curState = DEAD;
}
/// set state to the one, before die got killed (revive it!)
void DieState::revive() {
  this->_curState = this->_formerState;
}
/// check, if die got killed
/**
 \returns true, if die got killed; else false
 */
bool DieState::gotKilled() {
  return (this->_curState == DEAD);
}
/// return current value (1, ..., 6) of die
size_t DieState::getValue() {
  return this->_state[this->_curState][VALUE];
}
size_t DieState::getCurrentState() {
  return this->_curState;
}
size_t DieState::getFormerState() {
  return this->_formerState;
}
/// return color (BLACK or WHITE) of die
PlayColor DieState::getColor() {
  return this->_color;
}
/// return x position
int DieState::x() {
  return this->_x;
}
/// return y position
int DieState::y() {
  return this->_y;
}

/// initialize evaluation (without proper move, only rating)
Evaluation::Evaluation(float rating)
    : rating(rating),
      move(Move()) {
}
/// initialize evaluation (with rating and move)
Evaluation::Evaluation(float rating, Move move)
    : rating(rating),
      move(move) {
}

bool Evaluation::less::operator()(const Evaluation& lhs, const Evaluation& rhs) const {
  if (lhs.rating < rhs.rating) {
    return true;
  } else {
    return false;
  }
}
  
Game::Game(GameConfig c)
  : _fields(9, std::vector< int >(9, 0)),
    _dice(18, DieState()),
    _mode(c.getPlayMode()),
    _aiDepth(c.getAiDepth()),
    _strategy(c.getAiStrategy()),
    _nextPlayer(WHITE),
    _state(IDLE) {
    this->_setup();
}

// implement the 'big 3' to get clean copies of a game object

Game::Game(const Game& other)
    : _fields(other._fields),
      _dice(other._dice),
      _mode(other._mode),
      _aiDepth(other._aiDepth),
      _strategy(other._strategy),
      _moveStack(other._moveStack),
      _moveStackPending(other._moveStackPending),
      _deathStack(other._deathStack),
      _deathStackPending(other._deathStackPending),
      _nextPlayer(other._nextPlayer),
      _state(other._state) {
}

Game& Game::operator=(const Game& other) {
  // protect against self-assignment
  if (this != &other) {
    this->_fields = other._fields;
    this->_dice = other._dice;
    this->_mode = other._mode;
    this->_aiDepth = other._aiDepth;
    this->_strategy = other._strategy;
    this->_moveStack = other._moveStack;
    this->_moveStackPending = other._moveStackPending;
    this->_deathStack = other._deathStack;
    this->_deathStackPending = other._deathStackPending;
    this->_nextPlayer = other._nextPlayer;
    this->_state = other._state;
  }
  return *this;
}

Game::~Game() {
}

/// setup board to starting conditions
void Game::_setup() {
  this->_nextPlayer = WHITE;
  // initialize white dice
  this->_dice[0] = DieState(0, 0, WHITE, 19);
  this->_dice[1] = DieState(1, 0, WHITE, 1);
  this->_dice[2] = DieState(2, 0, WHITE, 5);
  this->_dice[3] = DieState(3, 0, WHITE, 22);
  this->_dice[4] = DieState(4, 0, WHITE, 24);
  this->_dice[5] = DieState(5, 0, WHITE, 22);
  this->_dice[6] = DieState(6, 0, WHITE, 5);
  this->_dice[7] = DieState(7, 0, WHITE, 1);
  this->_dice[8] = DieState(8, 0, WHITE, 19);
  // initialize black dice
  this->_dice[9] = DieState(0, 8, BLACK, 17);
  this->_dice[10] = DieState(1, 8, BLACK, 3);
  this->_dice[11] = DieState(2, 8, BLACK, 7);
  this->_dice[12] = DieState(3, 8, BLACK, 23);
  this->_dice[13] = DieState(4, 8, BLACK, 24);
  this->_dice[14] = DieState(5, 8, BLACK, 23);
  this->_dice[15] = DieState(6, 8, BLACK, 7);
  this->_dice[16] = DieState(7, 8, BLACK, 3);
  this->_dice[17] = DieState(8, 8, BLACK, 17);
  // initialize fields with ids of dice (or clear)
  this->clearBoard();
  for (size_t x = 0; x <= 8; x++) {
    this->_fields[x][0] = x;
  }
  for (size_t x = 0; x <= 8; x++) {
    this->_fields[x][8] = x + 9;
  }
}

void Game::clearBoard(){
  for (size_t y = 0; y < 9; y++) {
    for (size_t x = 0; x < 9; x++) {
      this->_fields[x][y] = CLEAR;
    }
  }
}

size_t Game::getNumberOfDice(){
  return this->_dice.size();
}

/// move die over board
void Game::makeMove(Move move, bool storeMove) {
  //TODO: check correctness! (Game::makeMove)
  KBX::Logger log("DieState");
  //// perform move
  DieState& dieState = this->_dice[move.dieIndex];
  log.info(stringprintf("initial state: %d", dieState.getCurrentState()));
  // delete die from current position on board
  this->_fields[dieState.x()][dieState.y()] = CLEAR;
  // get directions for horizontal movement (aka x coordinate)
  int directionX, directionY;
  if (move.rel.dx < 0) {
    directionX = WEST;
  } else {
    directionX = EAST;
  }
  // get directions for vertical movement (aka y coordinate)
  if (move.rel.dy < 0) {
    directionY = SOUTH;
  } else {
    directionY = NORTH;
  }
  // assume move to go first in y direction
  int stepsFirst = abs(move.rel.dy);
  int directionFirst = directionY;
  int stepsSec = abs(move.rel.dx);
  int directionSec = directionX;
  // swap values if move goes in x direction first
  if (move.rel.firstX) {
    swap(stepsFirst, stepsSec);
    swap(directionFirst, directionSec);
  }
  // traverse through dice states
  for (size_t i = stepsFirst; i > 0; i--) {
    // rotate in first direction
    dieState.moveOneStep(directionFirst);
    log.info(stringprintf("new state: %d", dieState.getCurrentState()));
  }
  for (size_t i = stepsSec; i > 0; i--) {
    // rotate in second direction
    dieState.moveOneStep(directionSec);
    log.info(stringprintf("new state: %d", dieState.getCurrentState()));
  }
  // delete old die on this position before moving new die to it
  int keyOldDie = this->_fields[dieState.x()][dieState.y()];
  if (keyOldDie != CLEAR) {
    this->_dice[keyOldDie].kill();
  }
  // if the move should be stored, do so
  if(storeMove){
    this->_moveStackPending.clear();
    this->_deathStackPending.clear();
    this->_moveStack.push_front(move);
    this->_deathStack.push_front(keyOldDie);
  }
  // move die to new position
  this->_fields[dieState.x()][dieState.y()] = move.dieIndex;
  this->_nextPlayer = inverse(this->_nextPlayer);
}

Move Game::undoMove() {
  if(_moveStack.empty()){
    return Move();
  }
  Move backMove = this->_moveStack.front();
  int victim = this->_deathStack.front();
  this->_moveStack.pop_front();
  this->_deathStack.pop_front();
  this->_moveStackPending.push_front(backMove);
  this->_deathStackPending.push_front(victim);
  backMove.rel = backMove.rel.invert();
  this->makeMove(backMove, false);
  if(victim != CLEAR){
    this->reviveDie(victim);
  }
  return backMove;
}

Move Game::redoMove() {
  //TODO: untested
  if(_moveStackPending.empty()) return Move();
  Move reMove = this->_moveStackPending.front();
  int victim = this->_deathStackPending.front();
  this->_moveStackPending.pop_front();
  this->_deathStackPending.pop_front();
  this->_moveStack.push_front(reMove);
  this->_deathStack.push_front(victim);
  this->makeMove(reMove, false);
  return reMove;
}

/// check if a given move is valid
/**
 \returns true, if the move is valid, else false
 */
bool Game::moveIsValid(Move move) {
  DieState dieState = this->_dice[move.dieIndex];
  // check, if die is from player with next move
  if (dieState.getColor() != this->_nextPlayer) {
    return false;
  }
  // check, if move is farer than die value allows
  if (dieState.getValue() != (size_t) (abs(move.rel.dx) + abs(move.rel.dy))) {
    return false;
  }
  // check, if move goes off the board
  if (dieState.x() + move.rel.dx < 0 || dieState.x() + move.rel.dx > 8 || dieState.y() + move.rel.dy < 0
      || dieState.y() + move.rel.dy > 8) {
    return false;
  }
  // check, if there are dice on the way, that cannot be crossed
  if (move.rel.firstX) {
    // iterate over x-values (before y-iteration)
    int end = abs(move.rel.dx);
    if (move.rel.dy == 0) {
      end--;
    }
    for (int i = 1; i <= end; i++) {
      if (this->_fields[dieState.x() + i * sgn(move.rel.dx)][dieState.y()] != CLEAR) {
        // there is a die on the way => move is not possible
        return false;
      }
    }
    // iterate over y-values
    for (int i = 1; i <= abs(move.rel.dy) - 1; i++) {
      if (this->_fields[dieState.x() + move.rel.dx][dieState.y() + i * sgn(move.rel.dy)] != CLEAR) {
        // there is a die on the way => move is not possible
        return false;
      }
    }
  } else { // first move in y direction
    int end = abs(move.rel.dy);
    if (move.rel.dx == 0) {
      end--;
    }
    // iterate over y-values first
    for (int i = 1; i <= end; i++) {
      if (this->_fields[dieState.x()][dieState.y() + i * sgn(move.rel.dy)] != CLEAR) {
        // there is a die on the way => move is not possible
        return false;
      }
    }
    // iterate over x-values (after y-iteration)
    for (int i = 1; i <= abs(move.rel.dx) - 1; i++) {
      int xVal = dieState.x() + i * sgn(move.rel.dx);
      int yVal = dieState.y() + move.rel.dy;
      //TODO: perform this boundary check for other field-lookups as well
      //TODO: think about above todo statement: isn't this done already above?
      if (0 <= xVal && xVal <= 8 && 0 <= yVal && yVal <= 8) {
        if (this->_fields[xVal][yVal] != CLEAR) {
          // there is a die on the way => move is not possible
          return false;
        }
      }
    }
  }
  // check target field
  int idTarget = this->_fields[dieState.x() + move.rel.dx][dieState.y() + move.rel.dy];
  if (idTarget != CLEAR) {
    // check if it is a dice of the enemy
    if (dieState.getColor() == this->_dice[idTarget].getColor()) {
      // dice have same color, cannot move to this field
      return false;
    }
  }
  // everything is ok, move possible
  return true;
}

Strategy& Game::getStrategy(){
  return this->_strategy;
}

/// get winner of game (black or white)
/**
 \returns black or white; null if nobody has won yet
 */
PlayColor Game::getWinner() {
  DieState& kingW = this->_dice[KING_WHITE];
  DieState& kingB = this->_dice[KING_BLACK];
  // check if king dice gets killed
  if (kingW.gotKilled()) {
    return BLACK;
  }
  if (kingB.gotKilled()) {
    return WHITE;
  }
  // winning by conquering opponents king field
  if ((kingW.x() == 4) && (kingW.y() == 8)) {
    return WHITE;
  }
  if ((kingB.x() == 4) && (kingB.y() == 0)) {
    return BLACK;
  }
  // nobody has won yet
  return NONE_OF_BOTH;
}
/// get player with next move
PlayColor Game::getNext() {
  return this->_nextPlayer;
}

PlayColor Game::getAiColor() {
  if (this->_mode == HUMAN_AI) {
    return BLACK;
  } else if (this->_mode == AI_HUMAN) {
    return WHITE;
  } else {
    return NONE_OF_BOTH;
  }
}

PlayColor Game::getHumanColor() {
  return inverse(this->getAiColor());
}


/// get die state of die with given id
DieState& Game::getDie(size_t id) {
  if (id < 18) {
    return this->_dice[id];
  } else {
    throw "index error: given index does not define a valid die";
  }
}
/// get die state of die with given coordinates
DieState& Game::getDie(size_t x, size_t y) {
  if (x < 9 && y < 9) {
    int id = this->_fields[x][y];
    return this->_dice[id];
  } else {
    throw "index error: given coordinates do not define a valid field";
  }
}

/// get die state of the die that was active last move
int Game::getLastActiveDie(){
  if(this->_moveStack.empty()){
    return CLEAR;
  }
  return this->_moveStack.front().dieIndex;
}

/// get die state of the die that died last move
int Game::getLastMovesVictim(){
  if(this->_deathStack.empty()){
    return CLEAR;
  }
  return this->_deathStack.front();
}

/// get die state of die with given coordinates
int Game::getDieId(size_t x, size_t y) {
  if (x < 9 && y < 9) {
    return this->_fields[x][y];
  } else {
    throw "index error: given coordinates do not define a valid field";
  }
}
/// rate the current game for a specified playcolor (black or white)
/**
 Rate the board for a specified side with all rating functions and
 value the single ratings based on the defined strategy.
 */
float Game::_rate(PlayColor color) {
  PlayColor winner;
  // check, if game is over
  winner = this->getWinner();
  // best/worst case if winning condition reached
  if (winner) {
    if (winner == color) {
      return 100.0f;
    } else {
      return -100.0f;
    }
  }
  float rating = 0.0f;
  rating += this->_strategy.coeffDiceRatio * this->_rateDiceRatio(color);
  return rating;
}
/// return list of all possible moves of selected die in current board setting
std::list< Move > Game::possibleMoves(size_t dieId) {
  std::list< Move > moves;
  int val = this->_dice[dieId].getValue();
  std::vector< RelativeMove >::const_iterator relMv;
  for (relMv = DieState::possibleMoves[val].begin(); relMv != DieState::possibleMoves[val].end(); relMv++) {
    Move mv = Move(dieId, *relMv);
    if (this->moveIsValid(mv)) {
      moves.push_back(mv);
    }
  }
  return moves;
}
/// return next evaluated move
Move Game::evaluateNext() {
  this->_state = EVALUATING;
  // aiDepth = number of human moves anticipated (hence times two because of response moves)
  Evaluation eval = this->_evaluateMoves(this->_aiDepth * 2, -100.0f, 100.0f, true);
  if (this->evaluating()) {
    this->_state = IDLE;
  }
  return eval.move;
}

/// print an evaluation
void Game::printEvaluation(const Evaluation& eval){
  DieState& die = this->getDie(eval.move.dieIndex);
  std::cout << "moving die "
            << eval.move.dieIndex
            << " from "
            << die.x()
            << "/"
            << die.y()
            << " to "
            << die.x()+eval.move.rel.dx
            << "/"
            << die.y()+eval.move.rel.dy
            << " received rating "
            << eval.rating
            << std::endl;
}

/// evaluate best possible move up to a certain level
/// this is done recursively by a form of the NegaMax algorithm with alpha-beta pruning
Evaluation Game::_evaluateMoves(int level, float alpha, float beta, bool initialCall) {
  KBX::Logger log("evaluation");
  if ((level == 0) || (this->getWinner() != NONE_OF_BOTH)) {
    return Evaluation(this->_rate(this->_nextPlayer));
  }
  // get rating, either directly or by recursive call
  float rating;
  // container for best move candidates
  std::priority_queue< Evaluation, std::vector< Evaluation >, Evaluation::less > candidates;
  // limit indices to significant color
  size_t from, to;
  if (this->_nextPlayer == WHITE) {
    from = 0;
    to = 8;
  } else {
    from = 9;
    to = 17;
  }
  // iterate over all dice of a color
  for (size_t d = from; d <= to; d++) {
    // get value of current die
    size_t value = this->_dice[d].getValue();
    // iterate over max number of moves for given dice value (stored in state-array)
    for (size_t i = 0; i < DieState::nPossibleMoves[value]; i++) {
      // abort evaluation if cancelled
      if (this->cancelled()) {
        return Evaluation(0.0f);
      }
      // check if this specific move is valid
      RelativeMove move = DieState::possibleMoves[value][i];
      if (this->moveIsValid(Move(d, move))) {
        // store all data to undo move later
        RelativeMove moveBack = move.invert();
        // kill the die lying on the target field
        int idDieOnTarget = this->_fields[this->_dice[d].x() + move.dx][this->_dice[d].y() + move.dy];
        // perform move
        this->makeMove(Move(d, move), false);
        // recursive call for next step (negative weighting, since it is opponent's turn)
        rating = - this->_strategy.patience * this->_evaluateMoves(level - 1, -beta, -alpha, false).rating;
        // undo move
        this->makeMove(Move(d, moveBack), false);
        // revive killed die on target field
        if (idDieOnTarget != CLEAR) {
          this->reviveDie(idDieOnTarget);
        }
        // alpha-beta pruning
        if (rating >= beta) {
          return Evaluation(rating);
        }
        if (rating > alpha) {
          alpha = rating;
          if (initialCall == true) {
            // add move to candidate list
            candidates.push(Evaluation(rating, Move(d, move)));
          }
        }
      }
    }
  }
  if (initialCall == true) {
    if ( ! candidates.empty()) {
      float topRating = candidates.top().rating;
      std::vector<Evaluation> topCandidates;
      log.info(stringprintf("top rating: %0.4f", topRating));
      while ((candidates.top().rating >= topRating) && ( ! candidates.empty())) {
        topCandidates.push_back(candidates.top());
        candidates.pop();
      }
      log.info(stringprintf("next rating: %0.4f", candidates.top().rating));
      log.info(stringprintf("no. of top candidates: %d", topCandidates.size()));
      // randomly select from equally rated top candidates
      std::size_t iTop = randomIndex(0, topCandidates.size()-1);
      return topCandidates[iTop];
    }
  }
  return Evaluation(alpha);
}

void Game::reviveDie(size_t idDieOnTarget){
  this->_dice[idDieOnTarget].revive();
  this->_fields[this->_dice[idDieOnTarget].x()][this->_dice[idDieOnTarget].y()] = idDieOnTarget;
}

/// reset the game
void Game::reset() {
  while(!this->_moveStack.empty()){
    this->_moveStack.pop_front();
    this->_deathStack.pop_front();
  }
  while(!this->_moveStackPending.empty()){
    this->_moveStackPending.pop_front();
    this->_deathStackPending.pop_front();
  }
  this->_setup();
}

void Game::setFinished() {
  this->_state = FINISHED;
}

bool Game::finished() {
  return (this->_state == FINISHED);
}

void Game::cancelEvaluation() {
  this->_state = CANCELLED;
}

bool Game::cancelled() {
  return (this->_state == CANCELLED);
}

bool Game::evaluating() {
  return (this->_state == EVALUATING);
}

PlayMode Game::playMode() {
  return this->_mode;
}

void Game::setPlayMode(PlayMode mode) {
  this->_mode = mode;
}

size_t Game::aiDepth() {
  return this->_aiDepth;
}

void Game::setAiDepth(size_t aiDepth) {
  this->_aiDepth = aiDepth;
}

/// rate dice ratio
/**
 rate according to the ratio of the number of dice on the board
 values:
 100%: only own dices on board
 0%:   only opponent's dices on board
 */
float Game::_rateDiceRatio(PlayColor color) {
  // default rating: 0.0; [-100.0, 100.0]
  float rating = 0.0f;
  for (size_t i = 0; i <= 17; i++) {
    if (this->_dice[i].gotKilled()) {
      // i from 0 to 8 are white dice, from 9 to 17 black dice
      if ((i <= 8 && color == WHITE) || (i >= 9 && color == BLACK)) {
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

void Game::printFields(){
  for(size_t i=0; i<9; i++){
    for(size_t j=0; j<9; j++){
      std::cout << _fields[i][j] << "\t";
    }
    std::cout << std::endl;
  }
}

} // end namespace KBX
