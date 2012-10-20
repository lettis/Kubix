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
#ifndef CONTROLLER__HPP
#define CONTROLLER__HPP

#include <map>
#include <vector>
#include <queue>

#include <QEvent>

#include "models.hpp"
#include "engine.hpp"

namespace KBX {
  /** 
      the controller loads the game and the board
      with all dice and sets these to the right fields with correct
      orientation. additionally, the dice (i.e. their graphical models)
      are connected to their model-counterparts (i.e. their abstract
      representation in the engine).
  */
  class Controller{
      Scene* _scene;
      Game*  _game;
      Board* _board;
      std::vector<Die*> _dice;
      std::map<size_t, size_t> _id2Die;
      std::map<size_t, std::pair<size_t,size_t> > _id2Field;
      PlayColor _nextTurn;
      int _markedId;
      int _selectedId;
  
      void _switchId(Activity activity, int& oldId, int newId);
      void _mark(int objectId);
      void _select(int objectId);
      void _markNext(int dx, int dy);
  public:
      Controller(Scene* scene, Game* game);
      int handle(QEvent* event);
  };
  
  // messaging
  /// define different message types
  enum MessageType {
       MARK_X_POS     = 0
      ,MARK_X_NEG     = 1
      ,MARK_Y_POS     = 2
      ,MARK_Y_NEG     = 3
      ,SELECT         = 4
      ,SELECT_GUI_OBJ = 5
  };
  /// more or less generic controller message
  class ControllerMessage{
  protected:
      void setupEmpty();
      QEvent event;
      QEvent userEvent;
  public:
      ControllerMessage();
      ControllerMessage(MessageType msgType);
      ControllerMessage(MessageType msgType, void* data);
      void send();
  };
  
  /// represent message for selection of gui element
  class SelectionMessage : public ControllerMessage{
      static std::queue<size_t> _idQueue;
  public:
      static size_t nextId();
      SelectionMessage(size_t id);
  };

} // end namespace KBX

#endif
