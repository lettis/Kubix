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

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "engine.hpp"

/** 
    the controller loads the game and the board
    with all dice and sets these to the right fields with correct
    orientation. additionally, the dice (i.e. their graphical models)
    are connected to their model-counterparts (i.e. their abstract
    representation in the engine).
*/
class KBX_Controller{
    KBX_Scene* _scene;
    KBX_Game*  _game;
    KBX_Board* _board;
    std::vector<KBX_Die*> _dice;
    std::map<size_t, size_t> _id2Die;
    std::map<size_t, std::pair<size_t,size_t> > _id2Field;
    KBX_PlayColor _nextTurn;
    int _markedId;

    void _mark(size_t objectId);
    void _markNext(int dx, int dy);
public:
    KBX_Controller(KBX_Scene* scene, KBX_Game* game);
    int handle(SDL_Event* event);
};

// messaging
/// define different message types
enum KBX_MessageType {
     MARK_X_POS     = 0
    ,MARK_X_NEG     = 1
    ,MARK_Y_POS     = 2
    ,MARK_Y_NEG     = 3
    ,SELECT         = 4
    ,SELECT_GUI_OBJ = 5
};
/// more or less generic controller message
class KBX_ControllerMessage{
protected:
    void setupEmpty();
    SDL_Event event;
    SDL_UserEvent userEvent;
public:
    KBX_ControllerMessage();
    KBX_ControllerMessage(KBX_MessageType msgType);
    KBX_ControllerMessage(KBX_MessageType msgType, void* data);
    void send();
};

/// represent message for selection of gui element
class KBX_SelectionMessage : public KBX_ControllerMessage{
    static std::queue<size_t> _idQueue;
public:
    static size_t nextId();
    KBX_SelectionMessage(size_t id);
};

#endif
