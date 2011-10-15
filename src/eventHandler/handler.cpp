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
#include <iostream>

#include "SDL.h"

#include "handler.hpp"
#include "gui.hpp"
#include "controller.hpp"

/// constructor for event handling classes
/**
    \param scene pointer to the scene
*/
KBX_EventHandler::KBX_EventHandler(KBX_Scene* scene){
    this->scene = scene;
}

/// constructor
KBX_ExitEventHandler::KBX_ExitEventHandler(KBX_Scene* scene) :
    KBX_EventHandler(scene)
{}
/// handle exit events
/**
    \param event the given event
    \returns 1 if exit event, else 0
*/
int KBX_ExitEventHandler::handle(SDL_Event* event){
    if (event->type == SDL_QUIT){
        return 1;
    }
    if (   event->type == SDL_KEYDOWN
        && event->key.keysym.sym == SDLK_ESCAPE){
        return 1;
    }
    return 0;
}

/// constructor
KBX_MotionEventHandler::KBX_MotionEventHandler(KBX_Scene* scene) :
    KBX_EventHandler(scene)
{ 
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    this->resize = false;
    this->keydown = false;
    this->fullscreen = false;
    this->zoom = 1;
    this->rotateHorizontal = 0;
    this->rotateVertical = 0;
    this->width = viewport[2];
    this->height = viewport[3];
    this->cameraDrag = false; 
}
/// handle motion events
/**
    \param event the given event
    \returns 1 if event has been handled, else 0
*/
int KBX_MotionEventHandler::handle(SDL_Event* event){
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
        float angle = 5.0;
        float zoom  = 0.05;
        this->keydown = true;
        switch(event->key.keysym.sym){
            case SDLK_a:
                this->rotateHorizontal =  angle;
                break;
            case SDLK_d:
                this->rotateHorizontal = -angle;
                break;
            case SDLK_w:
                this->rotateVertical =  angle;
                break;
            case SDLK_s:
                this->rotateVertical = -angle;
                break;
            case SDLK_q:
                this->zoom = 1.0 + zoom;
                break;
            case SDLK_e:
                this->zoom = 1.0 - zoom;
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
                SDL_Quit();
                initSDL(this->width, this->height, this->fullscreen);
                initOpenGL(this->width, this->height);
                loadTextures();
                break;
            default:
                // do nothing
                break;
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN){
        float angle = 0.2;
        switch (event->button.button){
            case SDL_BUTTON_WHEELUP:
                this->scene->zoom( 0.95 );
                break;
            case SDL_BUTTON_WHEELDOWN:
                this->scene->zoom( 1.05 );
                break;
            case SDL_BUTTON_RIGHT: 
                this->cameraDrag       = true;
                this->rotateHorizontal = angle;
                this->rotateVertical   = angle;
                break;
            default: 
                // do nothing
                break;
        }
    } else if (event->type == SDL_MOUSEBUTTONUP){
        switch (event->button.button){
            case SDL_BUTTON_RIGHT: 
                this->cameraDrag       = false;
                this->rotateHorizontal = 0;
                this->rotateVertical   = 0;
            default: 
                break;
        }
    } else if (event->type == SDL_MOUSEMOTION){
        int border = 1;
        if(    event->motion.x < border
            || event->motion.x > this->width-border-1
            || event->motion.y < border
            || event->motion.y > this->height-border-1){
            SDL_WM_GrabInput( SDL_GRAB_OFF );
            //TODO: is this below still needed? it was commented out, anyway. if not needed: delete
            //            setSDLWindow(this->width, this->height, false);
        } 
        if(this->cameraDrag){
            // actually perform the rotation
            this->scene->rotate(event->motion.xrel*this->rotateHorizontal, KBX_Camera::HORIZONTAL);
            this->scene->rotate(event->motion.yrel*this->rotateVertical, KBX_Camera::VERTICAL);
        }
    //TODO: what are we doing with these if-statements below?
    //TODO: if cout statements are needed, use logger instead
    } else if (event->type == SDL_VIDEOEXPOSE ){
        //std::cout << "videoexpose event" << std::endl;
    } else if (event->type == SDL_SYSWMEVENT ){
        //std::cout << "sys-wm event" << std::endl;
    } else if (event->type == SDL_ACTIVEEVENT ){
        if(event->active.gain == 1){
            SDL_WM_GrabInput( SDL_GRAB_ON );
        }
    }
    return 0;
}
/// apply the changes
/**
   this construction is necessary due to handling of KEYDOWN and KEYUP events.
   since only one KEYDOWN event is produced per key press,
   we have to poll for KEYUP events in order to determine if a key is still pressed.
   in the mean time, we want to apply the changes - otherwise the scene "freezes"
   until the user releases the key!
 */
void KBX_MotionEventHandler::proceed(){
    if(this->resize){
        setSDLWindow(this->width, this->height);
        setGLWindow(this->width, this->height);
        this->resize = false;
    }
    if(this->keydown){
        this->scene->zoom( this->zoom );
        this->scene->rotate( this->rotateHorizontal, KBX_Camera::HORIZONTAL );
        this->scene->rotate( this->rotateVertical, KBX_Camera::VERTICAL );
    }
}

/// initialize new selection event handler
KBX_SelectionEventHandler::KBX_SelectionEventHandler(KBX_Scene* scene) :
     KBX_EventHandler(scene)
    ,selectedObj(NULL)
{}
/// handle selection events
/**
    \param event the given event
    \returns 1 if event has been handled, else 0
*/
int KBX_SelectionEventHandler::handle(SDL_Event* event){
    if (event->type == SDL_MOUSEBUTTONDOWN){
        switch (event->button.button){
            case SDL_BUTTON_LEFT: 
                // get new object reference
                this->selectedObj = this->getObject(event->button.x, event->button.y);
                if(this->selectedObj){
                    // if object has been hit, send appropriate message to controller
                    KBX_SelectionMessage( this->selectedObj->id ).send();
                    return 1;
                } else {
                    return 0;
                }
            default:
                return 0;
        }
    }
    //TODO: handle cursor keys for selection (msg to controller)
    return 0;
}
/// get Object at given mouse coordinates
/**
    \param x horizontal mouse coordinate
    \param y vertical mouse coordinate
    \returns KBX_Object* to object under mouse cursor
*/
KBX_Object* KBX_SelectionEventHandler::getObject( size_t x, size_t y ){
    // get resolution from settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // get color information from frame buffer
    unsigned char pixel[3];
    this->scene->display(true);
    //TODO: 'so we have to change this': is this a TODO, or is it done with the code below?
    // Important: gl (0,0) is bottom left but window coords (0,0) are top left so we have to change this!      
    glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    size_t id = KBX_Color(pixel[0], pixel[1], pixel[2]).id();
    return KBX_Object::objectList.get(id);
}

/// constructor
KBX_ConsoleEventHandler::KBX_ConsoleEventHandler(KBX_Scene* scene) :
    KBX_EventHandler(scene)
{
    this->active = false;
}
/// handle console events
/**
    \param event the given event
    \returns 1 if event has been handled, else 0
*/
int KBX_ConsoleEventHandler::handle(SDL_Event* event){
    if (event->type == SDL_KEYDOWN){
        if(!this->active && event->key.keysym.sym == SDLK_RETURN){
            this->active = true;
            return 1;
        } else if(this->active){
            if(event->key.keysym.sym == SDLK_RETURN){
                this->scene->setText(input.str());
                this->input.clear();
                this->input.str("");
                this->active = !this->active;
                return 1;
            } else if ( (event->key.keysym.unicode < 0x80) && ( event->key.keysym.unicode > 0)){
                this->input << (char)event->key.keysym.unicode; 
                return 1;
            }
        }
    }
    return 0;
}

