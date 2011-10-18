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
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <string>
#include <map>

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "handler.hpp"
#include "engine.hpp"
#include "controller.hpp"

int main(){
    // enable logging
    KBX_Logger::enableInfos();
    KBX_Logger::enableWarnings();
    KBX_Logger::enableErrors();

//    KBX_Logger::filter( "KBX_Controller::_switchId" );

    // build logger instance for main
    KBX_Logger mainLog("main");
    try{
        // TODO: get config from cmd arguments + rc-files
        // define standard config
        KBX_Config config( HUMAN_AI, 4, KBX_Strategy(1.0) );
        // initialize SDL
        initSDL(800, 600, false);
        // initialize OpenGL
        initOpenGL(800, 600);
        // load the textures
        loadTextures();
        // initialize scene
        KBX_Scene* scene = new KBX_Scene();
        // initialize AI
        KBX_Game* game = new KBX_Game( config );
        // initialize controller (connection between view (scene) and model (game-engine))
        KBX_Controller controller(scene, game);
        // initialize event handlers
        KBX_ExitEventHandler      exitEvents(scene);
        KBX_MotionEventHandler    motionEvents(scene);
        KBX_SelectionEventHandler selectionEvents(scene);
        KBX_ConsoleEventHandler   consoleEvents(scene);
        // enter event loop
        SDL_Event* event = new SDL_Event();
        bool done=false;
        bool handled;
        while ( !done ){
            // get next event
            while ( SDL_PollEvent( event ) ){
                handled = false;
                // handle possible exit events
                if (exitEvents.handle( event ) != 0){
                    done=true;
                    handled = true;
                    break;
                }
                // handle possible console events
                if(!handled){
                    handled = consoleEvents.handle( event );
                }
                // handle possible selection events
                if(!handled){
                    handled = selectionEvents.handle( event );
                }
                // handle possible motion events
                if(!handled){
                    handled = motionEvents.handle( event );
                }

                // handle events generated especially for the controller
                if(!handled){
                    controller.handle( event );
                }
            }
            motionEvents.proceed();
            // redraw scene
            scene->display(false);
            SDL_GL_SwapBuffers();
        }
    }catch(const char* errMsg){
        mainLog.error( std::string(errMsg) );
    }
    SDL_Quit();
    return 0;
}

