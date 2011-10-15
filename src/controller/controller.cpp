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

#include <sstream>
#include <stdio.h>

#include "SDL.h"
#include "SDL_opengl.h"

#include "controller.hpp"
#include "gui.hpp"
#include "engine.hpp"

/// constructor
/**
    \param scene pointer to the scene object (view)
    \param game pointer to the game object (model)
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

    // setup dice with correct orientation.
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
    // add mapping from id of board-tile (gui representation) to
    // its xy-coordinates
    for (size_t i=0; i < 9; i++){
        for (size_t j=0; j < 9; j++){
            std::pair<size_t,size_t> xy(i,j);
            this->_id2Field[ this->_board->getTileId(i,j) ] = xy;
        }
    }
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
    size_t objectId;
    KBX_Logger log("controller handle");
    if (event->type == SDL_USEREVENT){
        switch( event->user.code ){
            case MARK_X_POS:
                log.info( "mark +x" );
                break;
            case MARK_X_NEG:
                log.info( "mark -x" );
                break;
            case MARK_Y_POS:
                log.info( "mark +y" );
                break;
            case MARK_Y_NEG:
                log.info( "mark -y" );
                break;
            case SELECT:
                log.info( "select" );
                break;
            case SELECT_GUI_OBJ:
                objectId = KBX_SelectionMessage::nextId();
                log.info( stringprintf("select gui obj: %d", objectId) );
                break;
            default:
                // do nothing
                break;
        }
    }
    return 0;
}

// messaging
/// initialize sdl event with common data
KBX_ControllerMessage::KBX_ControllerMessage(){
    this->setupEmpty();
}
/// initialize sdl event with specified message type
KBX_ControllerMessage::KBX_ControllerMessage(KBX_MessageType msgType){
    this->setupEmpty();
    this->userEvent.code = msgType;
}
/// initialize sdl event with specified message type and data
KBX_ControllerMessage::KBX_ControllerMessage(KBX_MessageType msgType, void* data){
    this->setupEmpty();
    this->userEvent.code = msgType;
    this->userEvent.data1 = data;
}
void KBX_ControllerMessage::setupEmpty(){
    // prepare empty user event
    this->userEvent.type = SDL_USEREVENT;
    this->userEvent.code = -1;
    this->userEvent.data1 = NULL;
    this->userEvent.data2 = NULL;
    // set event type to user event
    this->event.type = SDL_USEREVENT;
}
/// send message to sdl queue
void KBX_ControllerMessage::send(){
    this->event.user = this->userEvent;
    SDL_PushEvent( &this->event );
}

/// initialize queue to temporarily store selected ids
std::queue<size_t> KBX_SelectionMessage::_idQueue;
/// initialize gui selection message
KBX_SelectionMessage::KBX_SelectionMessage(size_t id) :
     KBX_ControllerMessage( SELECT_GUI_OBJ )
{
    KBX_SelectionMessage::_idQueue.push( id );
}
/// get next id
size_t KBX_SelectionMessage::nextId(){
    size_t id = KBX_SelectionMessage::_idQueue.front();
    KBX_SelectionMessage::_idQueue.pop();
    return id;
}
