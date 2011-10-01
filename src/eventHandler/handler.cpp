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

void list_hits(GLint hits, GLuint *names){
    int i;
    /*
      For each hit in the buffer are allocated 4 bytes:
      1. Number of hits selected (always one,
      beacuse when we draw each object
      we use glLoadName, so we replace the
      prevous name in the stack)
      2. Min Z
      3. Max Z
      4. Name of the hit (glLoadName)
    */
    printf("%d hits:\n", hits);
    for (i = 0; i < hits; i++)
	printf(	"Number: %d\n"
		"Min Z: %d\n"
		"Max Z: %d\n"
		"Name on stack: %d\n",
		(GLubyte)names[i * 4],
		(GLubyte)names[i * 4 + 1],
		(GLubyte)names[i * 4 + 2],
		(GLubyte)names[i * 4 + 3]
		);
	
    printf("\n");
}

void gl_selall(GLuint *buff){
    GLuint *p;
    int i;
    p = buff;
    for (i = 0; i < 6 * 4; i++){
	printf("Slot %d: - Value: %d\n", i, p[i]);
    }
    printf("Buff size: %x\n", (GLbyte)buff[0]);
}

void KBX_MotionEventHandler::select(int x, int y){
    GLuint buff[64] = {0};
    GLint hits, view[4];
    // This choose the buffer where store the values for the selection data
    glSelectBuffer(64, buff);
    // This retrieves info about the viewport
    glGetIntegerv(GL_VIEWPORT, view);
    // Switching in selecton mode
    glRenderMode(GL_SELECT);
    // Clearing the names' stack
    // This stack contains all the info about the objects
    glInitNames();
    // Now fill the stack with one element (or glLoadName will generate an error)
    glPushName(0);
    // Now modify the viewing volume, restricting selection area around the cursor
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // restrict the draw to an area around the cursor
    gluPickMatrix(x, y, 5.0, 5.0, view);
    gluPerspective(60, (float)view[2]/(float)view[3], 0.0001, 1000.0);
    // Draw the objects onto the screen
    glMatrixMode(GL_MODELVIEW);
    // draw only the names in the stack, and fill the array
    this->scene->display();
    // Do you remeber? We do pushMatrix in PROJECTION mode
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    // get number of objects drawn in that area
    // and return to render mode
    hits = glRenderMode(GL_RENDER);

    // Print a list of the objects
    list_hits(hits, buff);
    // comment/uncomment the following line to hide/show the whole buffer
    //    gl_selall(hits, buff);

    glMatrixMode(GL_MODELVIEW);
}

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
	case SDL_BUTTON_LEFT: 
	    printf("left mouse button pressed at %d %d\n", event->button.x, event->button.y);
	    // get resolution from settings
	    GLint view[4];
	    glGetIntegerv(GL_VIEWPORT, view);
	    //Important: gl (0,0) ist bottom left but window coords (0,0) are top left so we have to change this!
	    this->select(event->button.x,view[3] - event->button.y); 
	    break;
	case SDL_BUTTON_WHEELUP:
	    this->scene->zoom( 0.95 );
	    break;
	case SDL_BUTTON_WHEELDOWN:
	    this->scene->zoom( 1.05 );
	    break;
	}
    }
    return 0;
}

