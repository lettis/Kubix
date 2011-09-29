#include <stdlib.h>
#include <math.h>
#include <iostream>

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"
#include "handler.hpp"

int main(){
    SDL_Event* event = new SDL_Event();
    bool DONE=false;

    initSDL();
    initOpenGL();

    try{
        KBX_Die* werrfel = new KBX_Die();

        KBX_Scene* scene = new KBX_Scene();
        scene->add( werrfel );


        KBX_ExitEventHandler exitEvents(scene);
        KBX_MotionEventHandler motionEvents(scene);

        scene->rotate( 30, KBX_Camera::HORIZONTAL );

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

