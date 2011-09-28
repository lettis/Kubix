#include <stdlib.h>
#include <math.h>
#include <iostream>

#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"

int main(){
    SDL_Event ev;
    bool DONE=false;
    
    initSDL();
    initOpenGL();

    try{
        KBX_Die* werrfel = new KBX_Die();

        KBX_Scene scene;
        scene.add( werrfel );

        while ( ! DONE ){
            SDL_PollEvent( &ev );

            switch( ev.type ){
                case SDL_QUIT:
                    DONE=true;
                    break;
                case SDL_KEYDOWN:
                    if (ev.key.keysym.sym == SDLK_ESCAPE){
                        DONE=true;
                    }
                    break;
                default:
                    // do nothing
                    break;
            }

            scene.display();
            SDL_GL_SwapBuffers();
        }
    }catch(const char* errMsg){
        std::cout << "error: " << errMsg << std::endl;
    }
    SDL_Quit();
    return 0;
}

