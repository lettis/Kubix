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
            case SDLK_a:
                this->scene->rotate(angle, KBX_Camera::HORIZONTAL);
                break;
            case SDLK_d:
                this->scene->rotate(-angle, KBX_Camera::HORIZONTAL);
                break;
            case SDLK_w:
                this->scene->rotate(angle, KBX_Camera::VERTICAL);
                break;
            case SDLK_s:
                this->scene->rotate(-angle, KBX_Camera::VERTICAL);
                break;
            case SDLK_q:
                this->scene->zoom( 0.95 );
                break;
            case SDLK_e:
                this->scene->zoom( 1.05 );
                break;
            default:
                // do nothing
                break;
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN){
        switch (event->button.button){
            case SDL_BUTTON_WHEELUP:
                this->scene->zoom( 0.95 );
                break;
            case SDL_BUTTON_WHEELDOWN:
                this->scene->zoom( 1.05 );
                break;
            case SDL_BUTTON_LEFT: 
                this->cameraDrag = true;
                break;
            default: 
                break;
        }
    } else if (event->type == SDL_MOUSEBUTTONUP){
        switch (event->button.button){
        case SDL_BUTTON_LEFT: 
            this->cameraDrag = false;
        default: 
            break;
        }
    } else if (this->cameraDrag && (event->type == SDL_MOUSEMOTION)){
        float anglePerPixel = 1;
        this->scene->rotate(anglePerPixel*event->motion.xrel, KBX_Camera::HORIZONTAL);
        this->scene->rotate(anglePerPixel*event->motion.yrel, KBX_Camera::VERTICAL);
    }
    return 0;
}

/// handle selection events
/**
    \param event the given event
    \returns 1 if event has been handled, else 0
*/
int KBX_SelectionEventHandler::handle(SDL_Event* event){
    if (event->type == SDL_MOUSEBUTTONDOWN){
        switch (event->button.button){
            case SDL_BUTTON_LEFT: 
                if(this->selected) this->selected->highlighted = false;
                this->selected = this->getObject(event->button.x, event->button.y);
                if(this->selected){
                    this->selected->highlighted = true;
                    return 1;
                } else {
                    return 0;
                }
        default:
            return 0;
        }
    }
    return 0;
}


/// get Object at given mouse coordinates
/**
    \param x horizontal mouse coordinate
    \param y vertical mouse coordinate
    \returns KBX_Object pointer to object under mouse cursor
*/
KBX_Object* KBX_SelectionEventHandler::getObject( size_t x, size_t y ){
    // get resolution from settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // get color information from frame buffer
    unsigned char pixel[3];
    this->scene->display(true);
    // Important: gl (0,0) ist bottom left but window coords (0,0) are top left so we have to change this!      
    glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    size_t id = KBX_Color(pixel[0], pixel[1], pixel[2]).id();
    return KBX_Object::objectList.get(id);
}

