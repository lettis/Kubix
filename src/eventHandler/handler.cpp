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

#include <iostream>

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
    float angle;
    // note: the actual camera manipulations are performed in the 
    // KBX_MotionEventHandler::proceed() function
    // this is neccessary to provide fast responses 
    // and intuitive behaviour at the same time
    // see there for details
    if(event->type == SDL_VIDEORESIZE){
        this->width = event->resize.w;
        this->height = event->resize.h;
        this->resize = true;
    } else if (event->type == SDL_KEYDOWN){
        angle = 5.0;
        this->keydown = true;
        switch(event->key.keysym.sym){
            case SDLK_a:
                this->rotateHorizontal = angle;
                break;
            case SDLK_d:
                this->rotateHorizontal = -angle;
                break;
            case SDLK_w:
                this->rotateVertical = angle;
                break;
            case SDLK_s:
                this->rotateVertical = -angle;
                break;
            case SDLK_q:
                this->zoom = 1.05;
                break;
            case SDLK_e:
                this->zoom = 0.95;
                break;
            default:
                // do nothing
                break;
        }
    } else if (event->type == SDL_KEYUP){
        this->keydown = false;
        switch(event->key.keysym.sym){
        case SDLK_a:
            this->rotateHorizontal = 0;
            break;
        case SDLK_d:
            this->rotateHorizontal = 0;
            break;
        case SDLK_w:
            this->rotateVertical = 0;
            break;
        case SDLK_s:
            this->rotateVertical = 0;
            break;
        case SDLK_q:
            this->zoom = 1;
            break;
        case SDLK_e:
            this->zoom = 1;
            break;
        case SDLK_F11:
            this->fullscreen = !this->fullscreen;
            setWindow(800, 600, this->fullscreen);
            break;
        default:
            // do nothing
            break;
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN){
        angle = 0.3;
        switch (event->button.button){
            case SDL_BUTTON_WHEELUP:
                this->scene->zoom( 0.95 );
                break;
            case SDL_BUTTON_WHEELDOWN:
                this->scene->zoom( 1.05 );
                break;
            case SDL_BUTTON_LEFT: 
                this->cameraDrag = true;
                this->clickPosX = event->button.x;
                this->clickPosY = event->button.y;
                // find out whether the user clicked the "upper"/"lower" or "left/right" part of the screen
                // that is, if the user wanted to grab the "front/back" or "left/right" part of the board
                // in order to find the rotation direction
                angle *= sgn(this->scene->getOrientation().y);
                if(this->clickPosY > this->height/2){
                    angle *= -1;
                }
                this->rotateHorizontal = angle;
                this->rotateVertical = -angle;
                break;
            default: 
                break;
        }
    } else if (event->type == SDL_MOUSEBUTTONUP){
        switch (event->button.button){
        case SDL_BUTTON_LEFT: 
            this->cameraDrag = false;
            this->rotateHorizontal = 0;
            this->rotateVertical = 0;
        default: 
            break;
        }
    } else if (this->cameraDrag && (event->type == SDL_MOUSEMOTION)){
        // actually perform the rotation
        this->scene->rotate(event->motion.xrel*this->rotateHorizontal, KBX_Camera::HORIZONTAL);
        this->scene->rotate(event->motion.yrel*this->rotateVertical, KBX_Camera::VERTICAL);
    }
    return 0;
}

/// apply the changes
/**
   this construction is neccessary due to handling of KEYDOWN and KEYUP events
   since only one KEYDOWN event is produced per key press
   we have to poll for KEYUP events in order to determine if a key is still pressed
   in the mean time, we want to apply changes - otherwise the scene "freezes"
   until the user releases the key!
 */
void KBX_MotionEventHandler::proceed(){
    if(this->resize){
        setWindow(this->width, this->height);
        glFlush();
        this->resize = false;
    }
    if(this->keydown){
        this->scene->zoom( this->zoom );
        this->scene->rotate( this->rotateHorizontal, KBX_Camera::HORIZONTAL);
        this->scene->rotate( this->rotateVertical, KBX_Camera::VERTICAL);
    }
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

