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

#include "handler.hpp"
#include "gui.hpp"

/// constructor for event handling classes
/**
    \param scene pointer to the scene
*/
KBX_EventHandler::KBX_EventHandler(KBX_Scene* scene){
    this->scene = scene;
}
/// handle exit events
/**
    \param event the given event
    \returns 1 if exit event, else 0
*/
int KBX_ExitEventHandler::handle(SDL_Event* event){
    if (event->type == SDL_QUIT){
        return 1;
    }
    if (event->type == SDL_KEYDOWN &&
        event->key.keysym.sym == SDLK_ESCAPE){
        return 1;
    }
    return 0;
}
/// handle motion events
/**
    \param event the given event
    \returns 1 if event has been handled, else 0
*/
int KBX_MotionEventHandler::handle(SDL_Event* event){
    float angle=5;
    if (event->type == SDL_KEYDOWN){
        switch(event->key.keysym.sym){
            case SDLK_LEFT:
            case SDLK_a:
                this->scene->rotate(angle, KBX_Camera::HORIZONTAL);
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                this->scene->rotate(-angle, KBX_Camera::HORIZONTAL);
                break;
            case SDLK_UP:
            case SDLK_w:
                this->scene->rotate(angle, KBX_Camera::VERTICAL);
                break;
            case SDLK_DOWN:
            case SDLK_s:
                this->scene->rotate(-angle, KBX_Camera::VERTICAL);
                break;
            default:
                // do nothing
                break;
        }
    }
    return 0;
}

