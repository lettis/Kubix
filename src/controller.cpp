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

#include "controller.hpp"
#include "gui.hpp"
#include "engine.hpp"


namespace KBX {
  /// constructor
  /**
      \param scene pointer to the scene object (view)
      \param game pointer to the game object (model)
  */
//  Controller::Controller(Scene* scene, Game* game) :
//       _scene(scene)
//      ,_game(game)
//      ,_nextTurn(WHITE)
//      ,_markedId(CLEAR)
//      ,_selectedId(CLEAR)
//  {}




///TODO: throw away
  /// switch id
  /**
      \param activity the activity (marked, selected, etc) for which the ids should be changed
      \param oldId reference to the old id
      \param newId the new id
  
      switch the gui-object ids for a given activity (marked, selected, etc). this method checks,
      if there has been an object that has been marked, selected, etc before, sets its state to default
      and highlights the new object, identified by newId.
  */
/*
  void Controller::_switchId(Activity activity, int& oldId, int newId){
      Logger log("Controller::_switchId");
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
          Object::objectList.get( oldId )->activityState = DEFAULT;
      }
      if (newId != CLEAR){
          // mark new obj
          Object::objectList.get( newId )->activityState = activity;
      }
      oldId = newId;
  }
*/


//TODO: throw away
//  /// mark new object, unmark previous one
//  void Controller::_mark(int objectId){
//      this->_switchId( MARKED, this->_markedId, objectId );
//  }
//  /// select new object, deselect previous one
//  void Controller::_select(int objectId){
//      this->_switchId( SELECTED, this->_selectedId, objectId );
//  }














//TODO: throw all this away ...


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
  /*
//  int Controller::handle( QEvent* event ){
//      //TODO: finish this
//      size_t objectId;
//      Logger log("Controller::handle");
//      if (event->type == SDL_USEREVENT){
//          switch( event->user.code ){
//              case MARK_X_POS:
//                  log.info( "mark +x" );
//                  this->_markNext( 1, 0 );
//                  break;
//              case MARK_X_NEG:
//                  log.info( "mark -x" );
//                  this->_markNext( -1, 0 );
//                  break;
//              case MARK_Y_POS:
//                  log.info( "mark +y" );
//                  this->_markNext( 0, 1 );
//                  break;
//              case MARK_Y_NEG:
//                  log.info( "mark -y" );
//                  this->_markNext( 0, -1 );
//                  break;
//              case SELECT:
//                  log.info( "select" );
//                  objectId = this->_markedId;
//                  this->_mark( CLEAR );
//                  this->_select( objectId );
//                  break;
//              case SELECT_GUI_OBJ:
//                  objectId = SelectionMessage::nextId();
//                  log.info( stringprintf("select gui obj: %d", objectId) );
//                  this->_mark( CLEAR );
//                  this->_select( objectId );
//                  break;
//              default:
//                  // do nothing
//                  break;
//          }
//      }
//      return 0;
//  }
  
  // messaging

  /// initialize sdl event with specified message type
  ControllerMessage::ControllerMessage(MessageType msgType){
      this->setupEmpty();
//      this->userEvent.code = msgType;
  }
  /// initialize sdl event with specified message type and data
  ControllerMessage::ControllerMessage(MessageType msgType, void* data){
      this->setupEmpty();
//      this->userEvent.code = msgType;
//      this->userEvent.data1 = data;
  }
  void ControllerMessage::setupEmpty(){
      // prepare empty user event
//      this->userEvent.type = SDL_USEREVENT;
//      this->userEvent.code = -1;
//      this->userEvent.data1 = NULL;
//      this->userEvent.data2 = NULL;
//      // set event type to user event
//      this->event.type = SDL_USEREVENT;
  }
  /// send message to sdl queue
  void ControllerMessage::send(){
//      this->event.user = this->userEvent;
      //SDL_PushEvent( &this->event );
  }
  
  /// initialize queue to temporarily store selected ids
  std::queue<size_t> SelectionMessage::_idQueue;
  /// initialize gui selection message
  SelectionMessage::SelectionMessage(size_t id) :
       ControllerMessage( SELECT_GUI_OBJ )
  {
      SelectionMessage::_idQueue.push( id );
  }
  /// get next id (from static queue)
  size_t SelectionMessage::nextId(){
      size_t id = SelectionMessage::_idQueue.front();
      SelectionMessage::_idQueue.pop();
      return id;
  }

  */

} // end namespace KBX
