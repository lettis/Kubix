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

#include <queue>

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
    ,_nextTurn(WHITE)
    ,_markedId(CLEAR)
    ,_selectedId(CLEAR)
{
    // TODO: x/y coordinates of gui and engine seem not to match

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
    for (size_t x=0; x < 9; x++){
        for (size_t y=0; y < 9; y++){
            std::pair<size_t,size_t> xy(x,y);
            this->_id2Field[ this->_board->getTileId(x,y) ] = xy;
        }
    }
}
/// mark next object on the board
/**
    \param dx relative coordinate in x direction depending on currently marked object
    \param dy relative coordinate in y direction depending on currently marked object
    
    mark next object on relative coordinates depending on the currently marked object.
    if there is no object marked, automatically mark the field in the middle.
*/
void KBX_Controller::_markNext(int dx, int dy){
    int x,y;
    if (this->_markedId == CLEAR){
        // if no field marked, set centre field marked
        x = 3;
        y = 2;
    } else {
        if ( this->_id2Field.count( this->_markedId ) == 1 ){
            // prev. marked obj. was a field
            std::pair<size_t,size_t> xy = this->_id2Field[ this->_markedId ];
            x = xy.first;
            y = xy.second;
        } else {
            // prev. marked obj. was a die
            size_t dieId = this->_id2Die[ this->_markedId ];
            KBX_DieState* die = this->_game->getDie( dieId );
            x = die->x();
            y = die->y();
        }
        // check boundaries
        if ( 0 <= x+dx && x+dx < 9 ){
            x += dx;
        }
        if ( 0 <= y+dy && y+dy < 9 ){
            y += dy;
        }
    }
    size_t id = this->_board->getTileId( x, y );
    // mark the object
    this->_mark( id );
}
/// switch id
/**
    \param activity the activity (marked, selected, etc) for which the ids should be changed
    \param oldId reference to the old id
    \param newId the new id

    switch the gui-object ids for a given activity (marked, selected, etc). this method checks,
    if there has been an object that has been marked, selected, etc before, sets its state to default
    and highlights the new object, identified by newId.
*/
void KBX_Controller::_switchId(KBX_Activity activity, int& oldId, int newId){
    KBX_Logger log("KBX_Controller::_switchId");
    // check, if die lies on coordinates of newId;
    // if yes: select die instead of field
    if (this->_id2Field.count( newId ) == 1){
        log.info( "field selected" );
        // selected object is a field
        size_t x = this->_id2Field[ newId ].first;
        size_t y = this->_id2Field[ newId ].second;
        int dieId = this->_game->getDieId(x, y);
        log.info( stringprintf("coords: %d,%d ; die-id: %d", x, y, dieId) );
        if (dieId != CLEAR){
            log.info( stringprintf( "there is a die on the field: %d", dieId ) );
            // there is a die on the selected field
            newId = this->_dice[ dieId ]->id;
        }
    }
    if (oldId != CLEAR){
        // unmark prev
        KBX_Object::objectList.get( oldId )->activityState = DEFAULT;
    }
    if (newId != CLEAR){
        // mark new obj
        KBX_Object::objectList.get( newId )->activityState = activity;
    }
    oldId = newId;
}
/// mark new object, unmark previous one
void KBX_Controller::_mark(int objectId){
    this->_switchId( MARKED, this->_markedId, objectId );
}
/// select new object, deselect previous one
void KBX_Controller::_select(int objectId){
    this->_switchId( SELECTED, this->_selectedId, objectId );
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
    //TODO: finish this
    size_t objectId;
    KBX_Logger log("KBX_Controller::handle");
    if (event->type == SDL_USEREVENT){
        switch( event->user.code ){
            case MARK_X_POS:
                log.info( "mark +x" );
                this->_markNext( 1, 0 );
                break;
            case MARK_X_NEG:
                log.info( "mark -x" );
                this->_markNext( -1, 0 );
                break;
            case MARK_Y_POS:
                log.info( "mark +y" );
                this->_markNext( 0, 1 );
                break;
            case MARK_Y_NEG:
                log.info( "mark -y" );
                this->_markNext( 0, -1 );
                break;
            case SELECT:
                log.info( "select" );
                objectId = this->_markedId;
                this->_mark( CLEAR );
                this->_select( objectId );
                break;
            case SELECT_GUI_OBJ:
                objectId = KBX_SelectionMessage::nextId();
                log.info( stringprintf("select gui obj: %d", objectId) );
                this->_mark( CLEAR );
                this->_select( objectId );
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
/// get next id (from static queue)
size_t KBX_SelectionMessage::nextId(){
    size_t id = KBX_SelectionMessage::_idQueue.front();
    KBX_SelectionMessage::_idQueue.pop();
    return id;
}
