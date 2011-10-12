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
#ifndef HANDLER__HPP
#define HANDLER__HPP

#include <sstream>
#include <string>

#include "SDL.h"

#include "gui.hpp"

/// general event handling class
class KBX_EventHandler{
protected:
    KBX_Scene* scene;
public:
    KBX_EventHandler(KBX_Scene* scene);
    virtual int handle(SDL_Event* event)=0;
};

/// handler for exit events
class KBX_ExitEventHandler : public KBX_EventHandler{
public:
    KBX_ExitEventHandler(KBX_Scene* scene);
    int handle(SDL_Event* event);
};

/// handler for motion events
class KBX_MotionEventHandler : public KBX_EventHandler{
    bool resize;
    bool keydown;
    bool fullscreen;
    float zoom;
    float rotateHorizontal;
    float rotateVertical;
    float width;
    float height;
    bool cameraDrag;
    float clickPosX;
    float clickPosY;
public:
    KBX_MotionEventHandler(KBX_Scene* scene);
    void proceed();
    int handle(SDL_Event* event);
};

/// handler for motion events
class KBX_SelectionEventHandler : public KBX_EventHandler{
public:
    KBX_Object* selectedObj;
    KBX_SelectionEventHandler(KBX_Scene* scene);
    int handle(SDL_Event* event);
    KBX_Object* getObject( size_t x, size_t y );
};

/// handler for text/console events
class KBX_ConsoleEventHandler : public KBX_EventHandler{
    std::stringstream input;
    bool active;
public:
    KBX_ConsoleEventHandler(KBX_Scene* scene);
    int handle(SDL_Event* event);
};
#endif
