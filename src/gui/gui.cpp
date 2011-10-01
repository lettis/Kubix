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
#include "tools.hpp"

/// constructor initializing the color
KBX_Color::KBX_Color(){
    this->r = 0;
    this->g = 0;
    this->b = 0;
}
KBX_Color::KBX_Color(float r, float g, float b){
    this->r = r;
    this->g = g;
    this->b = b;
}

/// constructor initializing nullvector
KBX_Vec::KBX_Vec(){
    this->x = 0;
    this->y = 0;
    this->z = 0;
}
/// constructor of the KBX_Vec class 
/**
   \param x x-corrdinate (left-right in standard view)
   \param y y-coordinate (up-down in standard view)
   \param z z-coordinate (front-back in standard view)
 */
KBX_Vec::KBX_Vec(float x, float y, float z){
    this->x = x;
    this->y = y;
    this->z = z;
}
/// calculate the euclidian norm
float KBX_Vec::norm(){
    float norm = 0;
    norm += this->x * this->x;
    norm += this->y * this->y;
    norm += this->z * this->z;
    return sqrt(norm);
}
/// normalize vector to length=1
/*
    \returns the normalized vector
*/
KBX_Vec KBX_Vec::normalize(){
    float norm = this->norm();
    if (norm != 0){
        return KBX_Vec(   this->x/norm
		                , this->y/norm
		                , this->z/norm
		       );
    }else{
        throw "div by zero in vector norm";
    }
}
/// scale vector by factor
/**
    \param a scaling factor
    scaling is done by multiplying every element of the vector by the factor a
*/
KBX_Vec KBX_Vec::scale(float a){
    return KBX_Vec( a*this->x, a*this->y, a*this->z );
}
/// add vector
/**
    \param v the vector to be added
*/
KBX_Vec KBX_Vec::add(KBX_Vec v){
    return KBX_Vec(   this->x + v.x
		            , this->y + v.y
		            , this->z + v.z
		   );
}
/// subtract vector
/**
    \param v the vector to be subtracted
*/
KBX_Vec KBX_Vec::sub(KBX_Vec v){
    return this->add( v.scale(-1) );
}
/// rotate vector around given axis
/**
    \param rotAxis the axis around which the vector is to be rotated
    \param angle angle of rotation, given in degrees
    \returns the rotated vector
*/
KBX_Vec KBX_Vec::rotate(KBX_Vec rotAxis, float angle){
    // first normalize rotation axis
    rotAxis = rotAxis.normalize();
    float n1=rotAxis.x;
    float n2=rotAxis.y;
    float n3=rotAxis.z;
    // convert angle from degrees to radians
    angle = angle * 2*M_PI / 360;
    // calculate cosine, (1-cosine), sine
    float c=cos(angle);
    float cc=1-c;
    float s=sin(angle);
    // calculate new coordinates by applying rotation matrix
    float x= this->x*(c+n1*n1*cc)
            +this->y*(n1*n2*cc-n3*s)
            +this->z*(n1*n3*cc+n2*s);
    float y= this->x*(n2*n1*cc+n3*s)
            +this->y*(c+n2*n2*cc)
            +this->z*(n2*n3*cc-n1*s); 
    float z= this->x*(n3*n1*cc-n2*s)
            +this->y*(n3*n2*cc+n1*s)
            +this->z*(c+n3*n3*cc);
    return KBX_Vec(x,y,z);
}
/// calculate the cross product
/**
    \param v the other vector
    \returns the cross product  (this X v)
*/
KBX_Vec KBX_Vec::cross(KBX_Vec v){
    KBX_Vec result;
    result.x = this->y*v.z - this->z*v.y;
    result.y = this->z*v.x - this->x*v.z;
    result.z = this->x*v.y - this->y*v.x;
    return result;
}
/// constructor initializing to pos=(0,0,0) and target=(0,0,-1)
KBX_Camera::KBX_Camera(){
    this->position = KBX_Vec(0,0,0);
    this->target = KBX_Vec(0,0,-1);
}
/// constructor initializing to given pos and target=(0,0,-1)
KBX_Camera::KBX_Camera(KBX_Vec pos){
    this->position = pos;
    this->target = KBX_Vec(0,0,-1);
}
/// constructor initializing to given pos and target
KBX_Camera::KBX_Camera(KBX_Vec pos, KBX_Vec target){
    this->position = pos;
    this->target = target;
}
/// update the OpenGL camera perspective
void KBX_Camera::updateView(){
    // set OpenGL camera
    gluLookAt( this->position.x, this->position.y, this->position.z,
               this->target.x  , this->target.y  , this->target.z,
               0,1,0
    );
}
/// set new target
/**
    \param target the target to be set (i.e. the point where to look at)
*/
void KBX_Camera::setTarget(KBX_Vec target){
    this->target = target;
}
/// set new position
/**
    \param position the new position of the camera
*/
void KBX_Camera::setPosition(KBX_Vec position){
    this->position = position;
}
/// set new camera position by rotating around target
/**
    \param angle angle of rotation [degrees]
    \param direction rotate horizontally or vertically
    angle may be positive or negative.
    positive means to the right (horizontal) or upwards (vertical).
    negative means to the left (horizontal) or downwards (vertical).
*/
void KBX_Camera::rotate(float angle, size_t direction){
    KBX_Vec v=this->position.sub( this->target );
    if (direction == this->HORIZONTAL){
        // rotate in horizontal plane, i.e. around the y-axis
        v = v.rotate( KBX_Vec(0,1,0), angle );
        this->position = this->target.add( v );
    }else if (direction == this->VERTICAL){
        // rotate in vertical plane, i.e. around the axis
        // orthogonal to the y-axis and the vector v.
        if (   (v.normalize().y < 0.99 && angle > 0)  
             ||(v.normalize().y > -0.99 && angle < 0) ){
            KBX_Vec ortho = v.cross( KBX_Vec(0,1,0) );
            v = v.rotate( ortho, angle );
            this->position = this->target.add( v );
        }
    }else{
        throw "cannot rotate in unknown direction";
    }
}
/// reset camera position by zooming in/out
/**
    \param factor floating point number > 0, defines the zoom factor
    zooming in:
        0 < factor < 1
    zooming out:
        1 < factor
*/
void KBX_Camera::zoom(float factor){
    if (factor <= 0){
        throw "cannot zoom by negative or zero zoom factor";
    }
    KBX_Vec diff = this->position.sub( this->target );
    diff = diff.scale( factor );
    this->position = this->target.add( diff );
}


/// default constructor
KBX_Object::KBX_Object() :_angle(0)
                         ,_isVisible(true)
                         ,_pos( KBX_Vec(0,0,0) ) {}
/// constructor setting the object's position
KBX_Object::KBX_Object(KBX_Vec pos) :_angle(0)
                                    ,_isVisible(true)
                                    ,_pos(pos) {}
/// set object rotation
void KBX_Object::rotate(KBX_Vec axis, float angle){
    this->_angle = angle;
    this->_rotAxis = axis;
}
/// set object translation
void KBX_Object::translate(KBX_Vec direction){
    this->_pos = direction;
}
/// actually rotate object (private, only called by 'display')
void KBX_Object::_rotate(){
    glRotatef( this->_angle, this->_rotAxis.x, this->_rotAxis.y, this->_rotAxis.z );
}
/// actually translate object (private, only called by 'display')
void KBX_Object::_translate(){
    glTranslatef( this->_pos.x, this->_pos.y, this->_pos.z );
}
/// display the object
void KBX_Object::display(){
    // save transformations done before
    glPushMatrix();
    this->_rotate();
    this->_translate();
    this->_render();
    // reload transformations done before
    glPopMatrix();
}

/// inherit parent constructor
KBX_AnimObject::KBX_AnimObject() :KBX_Object() {}
/// inherit parent constructor
KBX_AnimObject::KBX_AnimObject(KBX_Vec pos) :KBX_Object(pos) {}

// this defines the keys for the die face textures
const size_t KBX_Die::FACE_K_W = 10;
const size_t KBX_Die::FACE_K_B = 20;
const size_t KBX_Die::FACE_1_W = 11;
const size_t KBX_Die::FACE_2_W = 12;
const size_t KBX_Die::FACE_3_W = 13;
const size_t KBX_Die::FACE_4_W = 14;
const size_t KBX_Die::FACE_5_W = 15;
const size_t KBX_Die::FACE_6_W = 16;
const size_t KBX_Die::FACE_1_B = 21;
const size_t KBX_Die::FACE_2_B = 22;
const size_t KBX_Die::FACE_3_B = 23;
const size_t KBX_Die::FACE_4_B = 24;
const size_t KBX_Die::FACE_5_B = 25;
const size_t KBX_Die::FACE_6_B = 26;
// tags for black/white
const size_t KBX_Die::WHITE = 1;
const size_t KBX_Die::BLACK = 2;
/// the textures of the die surfaces are handled "globally" by this static member
TextureHandler KBX_Die::textures = TextureHandler();
/// inherit parent constructor and set color
KBX_Die::KBX_Die(KBX_Vec pos, size_t color) :KBX_AnimObject(pos)
                                            ,_color(color) {}
/// render the die
void KBX_Die::_render(){
    // setting the color is neccessary in order to ensure that the texture is drawn 'as-is'
    // leaving this out might cause the texture to be drawn with 'shaded' colors
    // because all texture-pixel rgb values are multiplied with the corresponding values 
    // of the current color before being drawn!
    glColor3f(1, 1, 1);
    glEnable( GL_TEXTURE_2D );
    // face 1
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_1_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_1_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,-0.5);
    glEnd();
    // face 2
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_2_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_2_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,-0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,-0.5,+0.5);
    glEnd();
    // face 3
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_3_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_3_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(-0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,-0.5,+0.5);
    glEnd();
    // face 4
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_4_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_4_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(+0.5,-0.5,+0.5);
    glEnd();
    // face 5
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_5_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_5_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,+0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
    glEnd();
    // face 6
    if (this->_color == KBX_Die::WHITE){
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_6_W ) );
    } else {
        glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_6_B ) );
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,+0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,+0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
    glEnd();
    glDisable( GL_TEXTURE_2D );
}

/// board constructor
KBX_Board::KBX_Board(size_t rows, size_t cols){
    this->nRows = rows;
    this->nCols = cols;
    // define tile colors
    KBX_Color black = KBX_Color(0, 0, 0);
    KBX_Color white = KBX_Color(1, 1, 1);
    KBX_Color tileColor;
    KBX_Vec tilePosition;
    // allocate memory for tiles
    this->tiles = (KBX_Tile**) malloc( this->nCols*this->nRows * sizeof(KBX_Tile*) );     
    // setup tiles to form a checkered layout
    for(size_t row=0; row<this->nRows; row++){
        for(size_t col=0; col<this->nRows; col++){
            tileColor = ( (row%2 + col%2)%2 == 0 ) ? black : white ;
            tilePosition = KBX_Vec(   (float)row - (float)(this->nRows)/2
                                     ,-0.5
                                     ,(float)col - (float)(this->nCols)/2
                           );
            this->tiles[row + this->nRows*col] = new KBX_Tile( tilePosition, tileColor );
        }
    }
}
/// display board by rendering every tile
void KBX_Board::_render(){
    for(size_t i=0; i<(this->nRows * this->nCols); i++){
	glLoadName(i);
        this->tiles[i]->display();
    }
}

/// tile constructor
KBX_Tile::KBX_Tile(KBX_Vec pos, KBX_Color color) : KBX_Object(pos)
                                                 , basicColor(color)
                                                 , activeColor(color) {}
/// render the tile
void KBX_Tile::_render(){
    glColor3f(this->activeColor.r, this->activeColor.g, this->activeColor.b);
    glBegin( GL_QUADS );
     // upper face
     glVertex3f(0.0, 0.0, 0.0);
     glVertex3f(1.0, 0.0, 0.0);
     glVertex3f(1.0, 0.0, 1.0);
     glVertex3f(0.0, 0.0, 1.0);
     // lower face
     glVertex3f(0.0, -0.1, 0.0);
     glVertex3f(1.0, -0.1, 0.0);
     glVertex3f(1.0, -0.1, 1.0);
     glVertex3f(0.0, -0.1, 1.0);
     // sides
     glVertex3f(0.0,  0.0, 0.0);
     glVertex3f(1.0,  0.0, 0.0);
     glVertex3f(1.0, -0.1, 0.0);
     glVertex3f(0.0, -0.1, 0.0);

     glVertex3f(0.0,  0.0, 1.0);
     glVertex3f(1.0,  0.0, 1.0);
     glVertex3f(1.0, -0.1, 1.0);
     glVertex3f(0.0, -0.1, 1.0);

     glVertex3f(0.0,  0.0, 0.0);
     glVertex3f(0.0,  0.0, 1.0);
     glVertex3f(0.0, -0.1, 1.0);
     glVertex3f(0.0, -0.1, 0.0);

     glVertex3f(1.0,  0.0, 0.0);
     glVertex3f(1.0,  0.0, 1.0);
     glVertex3f(1.0, -0.1, 1.0);
     glVertex3f(1.0, -0.1, 0.0);
    glEnd();
}

/// scene constructor
KBX_Scene::KBX_Scene() :cam( KBX_Vec(0,0,-100), KBX_Vec(0,0,0) ) {}
/// render the scene
void KBX_Scene::_render(){
    // clear the graphics buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW); 
    // reset the drawing perspective
    glLoadIdentity();
    // set correct camera position/view
    this->cam.updateView();
    // call every object's display method to draw
    // the object to the scene
    for (size_t i=0; i<this->objList.size(); i++){
	glLoadName(i);
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
/// rotate the scene
/**
    \param angle the angle around which the scene should be rotated
    \param direction the direction of the rotation, either KBX_Camera.HORIZONTAL or KBX_Camera.VERTICAL
*/
void KBX_Scene::rotate(float angle, size_t direction){
    this->cam.rotate( angle, direction );
}
/// zoom the scene in/out
/**
    \param factor the zoom factor
*/
void KBX_Scene::zoom(float factor){
    this->cam.zoom( factor );
}

/// initialize sdl screen
void initSDL(){
    SDL_Surface *screen;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        throw stringprintf("Can't init SDL:  %s\n", SDL_GetError()).c_str();
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(800, 600, 16, SDL_OPENGL);
    if (screen == NULL) {
        throw stringprintf("Can't set video mode: %s\n", SDL_GetError()).c_str();
    }
}

/// initialize opengl
void initOpenGL(){
    // get resolution from settings
    GLint view[4];
    glGetIntegerv(GL_VIEWPORT, view);
    int w = view[2];
    int h = view[3];
    // TODO: implement fullscreen
    // handle window size correctly
    // TODO: this doesnt work
    glViewport(0,0, w, h);
    glMatrixMode(GL_PROJECTION); // switch to setting the camera perspective
    glLoadIdentity(); // reset the camera
    gluPerspective(	10.0,                  // the camera distance
                    (double)w / (double)h, // the width-to-height ratio
                    1.0,                   // the near z clipping coordinate
                    200.0);                // the far z clipping coordinate
    // use double buffering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // use grey background
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    // use smooth shading model
    glShadeModel(GL_SMOOTH);
    // draw objects respecting depth
    glEnable(GL_DEPTH_TEST);
}

