#include <stdlib.h>
#include <math.h>
#include <iostream>

#include "SDL.h"
#include "SDL_opengl.h"
#include "gui.hpp"

void loadTexture(char* filename){
	GLuint  texture;	// This is a handle to our texture object
	SDL_Surface *surface;	// This surface will tell us the details of the image
	GLenum texture_format;
	GLint  nOfColors;
	 
	if ( (surface = SDL_LoadBMP(filename)) ) { 
	 
		// Check that the image's width is a power of 2
		if ( (surface->w & (surface->w - 1)) != 0 ) {
			printf("warning: image width is not a power of 2\n");
		}
	 
		// Also check if the height is a power of 2
		if ( (surface->h & (surface->h - 1)) != 0 ) {
			printf("warning: image height is not a power of 2\n");
		}
	 
	        // get the number of channels in the SDL surface
	        nOfColors = surface->format->BytesPerPixel;
	        if (nOfColors == 4)     // contains an alpha channel
	        {
	                if (surface->format->Rmask == 0x000000ff)
	                        texture_format = GL_RGBA;
	                else
	                        texture_format = GL_BGRA;
	        } else if (nOfColors == 3)     // no alpha channel
	        {
	                if (surface->format->Rmask == 0x000000ff)
	                        texture_format = GL_RGB;
	                else
	                        texture_format = GL_BGR;
	        } else {
	                printf("warning: the image is not truecolor..  this will probably break\n");
	                // this error should not go unhandled
	        }
	 
		// Have OpenGL generate a texture object handle for us
		glGenTextures( 1, &texture );

		// Bind the texture object
		glBindTexture( GL_TEXTURE_2D, texture );
	 
		// Set the texture's stretching properties
	        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	 
		// Edit the texture object's image data using the information SDL_Surface gives us
		glTexImage2D( GL_TEXTURE_2D, 0, nOfColors, surface->w, surface->h, 0,
	                      texture_format, GL_UNSIGNED_BYTE, surface->pixels );
	} 
	 
	// Free the SDL_Surface only if it was successfully created
	if ( surface ) { 
		SDL_FreeSurface( surface );
	}
}


KBX_Vec::KBX_Vec(){
	this->x = 0;
	this->y = 0;
	this->z = 0;
}

float KBX_Vec::norm(){
	float norm = 0;
	norm += this->x * this->x;
	norm += this->y * this->y;
	norm += this->z * this->z;
	return sqrt(norm);
}

void KBX_Vec::normalize(){
	float norm = this->norm();
    if (norm != 0){
	    this->x /= norm;
	    this->y /= norm;
	    this->z /= norm;
    }else{
        throw "div by zero in vector norm";
    }
}
// -- KBX_Vec

// default constructor
KBX_Object::KBX_Object(){
	this->_angle = 0;
	this->_isVisible = true;
}

// set object rotation
void KBX_Object::rotate(KBX_Vec axis, float angle){
	this->_angle = angle;
	this->_rotAxis = axis;
}

// set object translation
void KBX_Object::translate(KBX_Vec direction){
	this->_pos = direction;
}

// actually rotate object (private, only called by 'display')
void KBX_Object::_rotate(){
	glRotatef( this->_angle, this->_pos.x, this->_pos.y, this->_pos.z );
}

// actually translate object (private, only called by 'display')
void KBX_Object::_translate(){
	glTranslatef( this->_pos.x, this->_pos.y, this->_pos.z );
}

void KBX_Object::display(){
	// save transformations done before
	glPushMatrix();
	this->_rotate();
	this->_translate();
	this->_render();
	// reload transformations done before
	glPopMatrix();
}
// -- KBX_Object

// --+ graphical objects +--
KBX_Die::KBX_Die() : KBX_AnimObject(){}

void KBX_Die::_render(){
	glBegin( GL_QUADS );

	// face 1
	 glColor3f(1,0,0);
	 glVertex3f(0,0,0);
	 glVertex3f(1,0,0);
	 glVertex3f(1,1,0);
	 glVertex3f(0,1,0);

	// face 2
	 glColor3f(0,1,0);
	 glVertex3f(0,0,0);
	 glVertex3f(1,0,0);
	 glVertex3f(1,0,1);
	 glVertex3f(0,0,1);

	// face 3
	 glColor3f(0,0,1);
	 glVertex3f(1,0,0);
	 glVertex3f(1,1,0);
	 glVertex3f(1,1,1);
	 glVertex3f(1,0,1);

	// face 4
	 glColor3f(0,0,1);
	 glVertex3f(0,0,0);
	 glVertex3f(0,1,0);
	 glVertex3f(0,1,1);
	 glVertex3f(0,0,1);

	// face 5
	 glColor3f(0,1,0);
	 glVertex3f(0,1,0);
	 glVertex3f(1,1,0);
	 glVertex3f(1,1,1);
	 glVertex3f(0,1,1);

	// face 6
	 glColor3f(1,0,0);
	 glVertex3f(0,0,1);
	 glVertex3f(1,0,1);
	 glVertex3f(1,1,1);
	 glVertex3f(0,1,1);

	glEnd();
}
// -- KBX_Die

void KBX_Board::_render(){
	// TODO: render board
}
// -- KBX_Board
/// scene constructor
KBX_Scene::KBX_Scene(){}
/// render the scene
void KBX_Scene::_render(){
    // clear the graphics buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW); 
    // reset the drawing perspective
    glLoadIdentity();
    // Set scene some pixels back to look ON it and not IN it
    glTranslatef( 0,0,-10 );
    // call every object's display method to draw
    // the object to the scene
    for (size_t i=0; i<this->objList.size(); i++){
        this->objList[i]->display();
    }
    // draw everything to screen
    SDL_GL_SwapBuffers();
}
/// add object to the scene
/**
    \param obj pointer to a KBX_Object to be added to the scene
    \throws string exception, if trying to add null-reference to scene
*/
void KBX_Scene::add(KBX_Object* obj){
    if (obj){
        this->objList.push_back( obj );
    }else{
        throw "unable to add object to scene (null-reference)";
    }
}


// --+ end graphical objects +--


// initialize sdl screen
void initSDL(){
        SDL_Surface *screen;
        if (SDL_Init(SDL_INIT_VIDEO) == -1) {
                printf("Can't init SDL:  %s\n", SDL_GetError());
                exit(1);
        }
        atexit(SDL_Quit);
        screen = SDL_SetVideoMode(800, 600, 16, SDL_OPENGL);
        if (screen == NULL) {
                printf("Can't set video mode: %s\n", SDL_GetError());
                exit(1);
        }
}

// initialize opengl
void initOpenGL(){
	//TODO: get resolution from settings and implement fullscreen
	int w = 800;
	int h = 600;
	// handle window size correctly
	// TODO: this doesnt work
	glViewport(0,0, w, h);
	glMatrixMode(GL_PROJECTION); // switch to setting the camera perspective
	// set the camera perspective
	glLoadIdentity(); // reset the camera
	gluPerspective(	45.0,                  // the camera angle
			(double)w / (double)h, // the width-to-height ratio
			1.0,                   // the near z clipping coordinate
			200.0);                // the far z clipping coordinate
	// TODO: set camera orientation
	//gluLookAt( ... );

	// use double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// use black background
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// use smooth shading model
        glShadeModel(GL_SMOOTH);
	// draw objects respecting depth
	glEnable(GL_DEPTH_TEST);
}

