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

#include "SDL.h"
#include "SDL_opengl.h"

#include "controller.hpp"
#include "gui.hpp"
#include "engine.hpp"

/// constructor
/**
    \param scene pointer to the scene object (view)
    \param game pointer to the game object (model)

    the controller constructor loads the game and the board
    with all dice and sets these to the right fields with correct
    orientation. additionally, the dice (i.e. their graphical models)
    are connected to their model-counterparts (i.e. their abstract
    representation in the engine).
*/
KBX_Controller::KBX_Controller(KBX_Scene* scene, KBX_Game* game) :
     _scene(scene)
    ,_game(game)
{
    // define rotation axes
    KBX_Vec toFront         (-1.0,  0.0,  0.0);
    KBX_Vec toBack          ( 1.0,  0.0,  0.0);
    KBX_Vec toLeft          ( 0.0,  0.0,  1.0);
    KBX_Vec toRight         ( 0.0,  0.0, -1.0);
    KBX_Vec clockwise       ( 0.0, -1.0,  0.0);
    KBX_Vec counterClockwise( 0.0,  1.0,  0.0);

    // setup dice with correct orientation
    // per definition, the dice are set up in the same order as the dice
    // are defined in the engine.
    // this way, the id of a single die in the _dice vector and in the engine
    // are the same. from this, the following order follows:
    //      white dice, from left to right (sight from behind white) have ids
    //      0, 1, 2, 3, 4 (king), 5, 6, 7, 8
    //      black dice, from left to right (sight from behind white) have ids
    //      9, 10, 11, 12, 13 (king), 14, 15, 16, 17.

    // white dice; w1 is in lower left corner, w8 in lower right
    this->_dice.push_back( new KBX_Die( KBX_Vec(-4,0, 4), WHITE ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-3,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-2,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-1,0, 4), WHITE ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 0,0, 4), WHITE, true ) );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 1,0, 4), WHITE ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 2,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 3,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 4,0, 4), WHITE ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    // black dice; b1 is in upper left corner, b8 in upper right
    this->_dice.push_back( new KBX_Die( KBX_Vec(-4,0,-4), BLACK ) );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-3,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-2,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec(-1,0,-4), BLACK ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 0,0,-4), BLACK, true ) );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 1,0,-4), BLACK ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 2,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 3,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new KBX_Die( KBX_Vec( 4,0,-4), BLACK ) );
    this->_dice.back()->rotate( clockwise, 90 );
    // add dice to scene
    for (size_t i=0; i < this->_dice.size(); i++){
        // add dice to scene
        scene->add( this->_dice[i] );
        // add mapping from id of gui-die to
        // internal id (here: i) of abstract representation
        this->_id2Die[ this->_dice[i]->id ] = i;
    }
    // initialize the board and add it to the scene
    this->_board = new KBX_Board(9, 9);
    scene->add( this->_board );
}
/// handle events especially generated for the controller
/**
    \param event the event generated for the controller
    \returns 1 if event got handled, else 0

    the idea is, that the other event handlers use the SDL message queue
    to send messages directly to the controller. that way, events that
    have to be handled further (object has been clicked) will be forwarded
    with more detailed information (id of the object).
    this means, the controller doesnt have to handle every gui event on it's own;
    instead the gui events are filtered and preprocessed by the other event handlers.
*/
int KBX_Controller::handle( SDL_Event* event ){
    //TODO: implement this
    return 0;
}
