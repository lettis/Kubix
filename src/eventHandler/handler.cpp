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

/// handle motionn events
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
//                this->scene->rotate( );
                break;
            default:
                // do nothing
                break;
        }
    }
    return 0;
}

