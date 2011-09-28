#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include "SDL_opengl.h"

#include "gui.hpp"

int main(){
	SDL_Event ev;
	bool DONE=false;
	
	initSDL();
	initOpenGL();

	KBX_Die* werrfel = new KBX_Die();
	
	while ( ! DONE ){

		SDL_PollEvent( &ev );
		if ( ev.type == SDL_QUIT || ev.key.keysym.sym == SDLK_ESCAPE ){
			DONE = true;
		}

		// TODO: this rendering stuff should rest in scene.display()
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
		glLoadIdentity(); //Reset the drawing perspective

		glTranslatef( 0,0,-10);

		//TODO: render stuff here
		werrfel->display();

		SDL_GL_SwapBuffers();
	}
	
	SDL_Quit();
	return 0;
}

