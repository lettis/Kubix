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

        while ( !DONE ){
            SDL_PollEvent( event );

            if (exitEvents.handle( event ) != 0){
                DONE=true;
            }

            scene->display();
            SDL_GL_SwapBuffers();
        }
    }catch(const char* errMsg){
        std::cout << "error: " << errMsg << std::endl;
    }
    SDL_Quit();
    return 0;
}

