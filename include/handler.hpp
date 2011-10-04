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
    KBX_ExitEventHandler(KBX_Scene* scene) :KBX_EventHandler(scene) {};
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
    KBX_MotionEventHandler(KBX_Scene* scene) :KBX_EventHandler(scene) { 
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        this->fullscreen = false;
        this->width = viewport[2];
        this->height = viewport[3];
        this->resize = false;
        this->keydown = false;
        this->zoom = 1;
        this->rotateHorizontal = 0;
        this->rotateVertical = 0;
        this->cameraDrag = false; 
    };
    void proceed();
    int handle(SDL_Event* event);
};

/// handler for motion events
class KBX_SelectionEventHandler : public KBX_EventHandler{
public:
    KBX_Object* selected;
    KBX_SelectionEventHandler(KBX_Scene* scene) :KBX_EventHandler(scene) { this->selected = NULL; };
    int handle(SDL_Event* event);
    KBX_Object* getObject( size_t x, size_t y );
};
#endif
