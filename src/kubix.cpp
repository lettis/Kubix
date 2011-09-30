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

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "handler.hpp"

int main(){
    // initialize SDL
    initSDL();
    // initialize OpenGL
    initOpenGL();
    try{
        // initialize scene
        KBX_Scene* scene = new KBX_Scene();
        // add die to scene
        KBX_Die* werrfel  = new KBX_Die( KBX_Vec(0,0,0)   );
        KBX_Die* werrfel2 = new KBX_Die( KBX_Vec(1,0,0) );
        scene->add( werrfel );
        scene->add( werrfel2 );
        // initialize event handlers
        KBX_ExitEventHandler exitEvents(scene);
        KBX_MotionEventHandler motionEvents(scene);
        // enter event loop
        SDL_Event* event = new SDL_Event();
        bool DONE=false;
        while ( !DONE ){
            // get next event
            SDL_PollEvent( event );
            // handle possible exit events
            if (exitEvents.handle( event ) != 0){
                DONE=true;
            }
            // handle possible motion events
            motionEvents.handle( event );
            // redraw scene
            scene->display();
            SDL_GL_SwapBuffers();
        }
    }catch(const char* errMsg){
        std::cout << "error: " << errMsg << std::endl;
    }
    SDL_Quit();
    return 0;
}

