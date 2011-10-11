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
#include "SDL_image.h"

#include <GL/glu.h>
#include <GL/glx.h>

#include "gui.hpp"
#include "tools.hpp"
// engine is needed for KBX_PlayColor definition
#include "engine.hpp"

// define fixed color values
const KBX_Color KBX_Color::BLACK(0.0f, 0.0f, 0.0f);
const KBX_Color KBX_Color::GREY10(0.1f, 0.1f, 0.1f);
const KBX_Color KBX_Color::GREY20(0.2f, 0.2f, 0.2f);
const KBX_Color KBX_Color::GREY30(0.3f, 0.3f, 0.3f);
const KBX_Color KBX_Color::GREY40(0.4f, 0.4f, 0.4f);
const KBX_Color KBX_Color::GREY50(0.5f, 0.5f, 0.5f);
const KBX_Color KBX_Color::GREY60(0.6f, 0.6f, 0.6f);
const KBX_Color KBX_Color::GREY70(0.7f, 0.7f, 0.7f);
const KBX_Color KBX_Color::GREY80(0.8f, 0.8f, 0.8f);
const KBX_Color KBX_Color::GREY90(0.9f, 0.9f, 0.9f);
const KBX_Color KBX_Color::WHITE(1.0f, 1.0f, 1.0f);
// define different versions of the KBX_Color constructor depending on parameters
KBX_Color::KBX_Color(){
    this->r = 0;
    this->g = 0;
    this->b = 0;
}
KBX_Color::KBX_Color(size_t id){
    this->r = float(id%255)/255;
    this->g = float((id/255)%255)/255;
    this->b = float((id/(255*255))%255)/255;
}
KBX_Color::KBX_Color(unsigned char r, unsigned char g, unsigned char b){
    this->r = float(r)/255;
    this->g = float(g)/255;
    this->b = float(b)/255;
}
KBX_Color::KBX_Color(int r, int g, int b){
    this->r = float(r)/255;
    this->g = float(g)/255;
    this->b = float(b)/255;
}
KBX_Color::KBX_Color(float r, float g, float b){
    this->r = r;
    this->g = g;
    this->b = b;
}
size_t KBX_Color::id() const {
    // TODO: think about this. this implementation leads to some pretty large ids
    //       if the color is bright (e.g. white: 1.0*255 + 1.0*255^2 + 1.0*255^3 ).
    //       is that necessary?
    return this->r*255 + 255*255*this->g + 255*255*255*this->b;
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
/// get orientation of camera
KBX_Vec KBX_Camera::getOrientation(){
  return this->position.sub( this->target );
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
    if(diff.norm() < 900 && diff.norm() > 15){
        this->position = this->target.add( diff );
    }
}

KBX_ObjectHandler KBX_Object::objectList;

/// default constructor
KBX_Object::KBX_Object() :
     _angle(0)
    ,_isVisible(true)
    ,_pos( KBX_Vec(0,0,0) )
    ,highlighted(false)
    // we need to add the object to the object list
    // and retrieve our own unique id from there
    ,id(KBX_Object::objectList.add(this)) 
{}
/// constructor setting the object's position
KBX_Object::KBX_Object(KBX_Vec pos) :
     _angle(0)
    ,_isVisible(true)
    ,_pos( pos )
    ,highlighted(false)
    // we need to add the object to the object list
    // and retrieve our own unique id from there
    ,id(KBX_Object::objectList.add(this)) 
{}

/// set object rotation (accumulatively)
/**
    \param axis the axis around which the object will be rotated
    \param angle angle of the rotation

    Rotations will be done accumulatively. This means, you can
    call .rotate(...) several times after another with different rotation
    axes and angles. The orientation will always be in the direction of the 
    first rotation. That way you can rotate the object with the same reference
    of orientation and you do not have to account for the rotating coordinate system.
    This is done by rotating the rotation axis inversely around the last rotation.
    Thus the reference frame is always the same.
*/
void KBX_Object::rotate(KBX_Vec axis, float angle){
    // push angle to list of successive rotations
    this->_angle.push_back( angle );
    // push axis to list of succ. rotations by converting it
    // to the local frame of reference
    // (this is needed, since the OpenGL coordinate system rotates with the objects)
    if ( !this->_rotAxis.empty() ){
        size_t last = this->_rotAxis.size()-1;
        axis = axis.rotate( this->_rotAxis[ last ].scale( -1 ), this->_angle[ last ] );
    }
    this->_rotAxis.push_back( axis );
}
/// set object translation
void KBX_Object::translate(KBX_Vec direction){
    this->_pos = direction;
}
/// actually rotate object (private, only called by 'display')
void KBX_Object::_rotate(){
    for(size_t i=0; i<this->_angle.size(); i++){
        glRotatef( this->_angle[i], this->_rotAxis[i].x, this->_rotAxis[i].y, this->_rotAxis[i].z );
    }
}
/// actually translate object (private, only called by 'display')
void KBX_Object::_translate(){
    glTranslatef( this->_pos.x, this->_pos.y, this->_pos.z );
}
/// display the object
void KBX_Object::display(bool picking){
    // save transformations done before
    glPushMatrix();
    this->_translate();
    this->_rotate();
    this->_render(picking);
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
/// the textures of the die surfaces are handled "globally" by this static member
TextureHandler KBX_Die::textures = TextureHandler();
/// inherit parent constructor and set color
KBX_Die::KBX_Die(KBX_Vec pos, KBX_PlayColor color) :
     KBX_AnimObject(pos)
    ,_playColor(color)
    ,IS_KING(false)
{}
KBX_Die::KBX_Die(KBX_Vec pos, KBX_PlayColor color, bool IS_KING) :
     KBX_AnimObject(pos)
    ,_playColor(color)
    ,IS_KING(IS_KING)
{}
/// render the die
void KBX_Die::_render(bool picking){
    // setting the color is neccessary in order to ensure that the texture is drawn 'as-is'
    // leaving this out might cause the texture to be drawn with 'shaded' colors
    // because all texture-pixel rgb values are multiplied with the corresponding values 
    // of the current color before being drawn!
    if(picking){
        KBX_Color pickerColor = KBX_Color(this->id);
        glColor3f(pickerColor.r, pickerColor.g, pickerColor.b);
    } else {
        if (this->highlighted){
            glColor3f(1.0f, 0.0f, 0.0f);
        } else {
            glColor3f(1.0, 1.0, 1.0);
        }
        glEnable( GL_TEXTURE_2D );
    }
    // prepare die face for king die if necessary
    GLuint kingFace;
    if (this->IS_KING){
        if (this->_playColor == WHITE){
            kingFace = KBX_Die::textures.get( KBX_Die::FACE_K_W );
        } else {
            kingFace = KBX_Die::textures.get( KBX_Die::FACE_K_B );
        }
    }
    // face 1
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_1_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_1_B ) );
        }
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,-0.5);
    glEnd();
    // face 2
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_2_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_2_B ) );
        }
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,-0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,-0.5,+0.5);
    glEnd();
    // face 3
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_3_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_3_B ) );
        }
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(-0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,-0.5,+0.5);
    glEnd();
    // face 4
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_4_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_4_B ) );
        }
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(+0.5,-0.5,+0.5);
    glEnd();
    // face 5
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_5_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_5_B ) );
        }
    }
    glBegin( GL_QUADS );
     glTexCoord2f(0.0,1.0); glVertex3f(-0.5,+0.5,-0.5);
     glTexCoord2f(1.0,1.0); glVertex3f(+0.5,+0.5,-0.5);
     glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
     glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
    glEnd();
    // face 6
    if(!picking){
        if (this->IS_KING){
            glBindTexture( GL_TEXTURE_2D, kingFace );
        } else if (this->_playColor == WHITE){
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_6_W ) );
        } else {
            glBindTexture( GL_TEXTURE_2D, KBX_Die::textures.get( KBX_Die::FACE_6_B ) );
        }
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
KBX_Board::KBX_Board(size_t rows, size_t cols) :
     _nRows(rows)
    ,_nCols(cols)
    ,_tiles(_nRows, std::vector<KBX_Tile*>(_nCols))
{
    // define tile colors
    KBX_Color dark = KBX_Color::GREY40;
    KBX_Color bright = KBX_Color::GREY60;
    KBX_Color tileColor;
    KBX_Vec tilePosition;
    // allocate memory for tiles
    //this->tiles = (KBX_Tile**) malloc( this->nCols*this->nRows * sizeof(KBX_Tile*) );     

    // setup tiles to form a checkered layout
    for(size_t row=0; row < this->_nRows; row++){
        for(size_t col=0; col < this->_nCols; col++){
            tileColor = ( (row%2 + col%2)%2 == 0 ) ? dark : bright ;
            tilePosition = KBX_Vec(   (float)row - (float)(this->_nRows)/2
                                     ,-0.5
                                     ,(float)col - (float)(this->_nCols)/2
                           );
            this->_tiles[row][col] = new KBX_Tile( tilePosition, tileColor );
        }
    }
}
/// free memory of allocated tiles
KBX_Board::~KBX_Board(){
    std::vector<KBX_Tile*>::iterator it;
    for (size_t r=0; r < this->_nRows; r++){
        for (it=this->_tiles[r].begin(); it < this->_tiles[r].end(); it++){
            delete *it;
        }
    }
}
/// display board by rendering every tile
void KBX_Board::_render(bool picking){
    for (size_t r=0; r < this->_nRows; r++){
        for (size_t c=0; c < this->_nCols; c++){
            glLoadName(r+c*this->_nCols);
            this->_tiles[r][c]->display(picking);
        }
    }
}

/// tile constructor
KBX_Tile::KBX_Tile(KBX_Vec pos, KBX_Color color) :
     KBX_Object(pos)
    ,basicColor(color)
    ,activeColor(color)
{}
/// render the tile
void KBX_Tile::_render(bool picking){
    if(picking){
	KBX_Color pickerColor = KBX_Color(this->id);
        glColor3f(pickerColor.r, pickerColor.g, pickerColor.b);
    } else if (this->highlighted){
        glColor3f(1.0f, 0.0f, 0.0f);
    } else { 
        glColor3f(this->activeColor.r, this->activeColor.g, this->activeColor.b);
    }
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

void KBX_Scene::setText(std::string s){
    this->textOpacity = 1.0f;
    this->text = s;
}

/// scene constructor
KBX_Scene::KBX_Scene() :cam( KBX_Vec(0,0,100), KBX_Vec(0,0,0) ) {
    this->font = glBuildFont();
    this->textOpacity = 0;
}

// scene destructor
KBX_Scene::~KBX_Scene(){
    glKillFont(this->font);
}

/// render the scene
void KBX_Scene::_render(bool picking){
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
        this->objList[i]->display(picking);
    }

    // draw the text
    if(this->textOpacity > 0.2f){
        glColor4f(this->textOpacity, this->textOpacity, this->textOpacity, this->textOpacity);
        this->textOpacity -= 0.005f;
        glPushMatrix();  
        glRasterPos3f(0,3,0);  
        glPrint(this->text.c_str(), this->font);
        glPopMatrix();
    }

    // draw everything to screen
    if(!picking) SDL_GL_SwapBuffers();
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

/// obtain camera orientation relative to center of scene
KBX_Vec KBX_Scene::getOrientation(){
  return this->cam.getOrientation();
}

/// zoom the scene in/out
/**
    \param factor the zoom factor
*/
void KBX_Scene::zoom(float factor){
    this->cam.zoom( factor );
}

/// initialize sdl screen
void initSDL(int width, int height, bool fullscreen){
    // Color depth in bits of our window.
    int bpp = 0;
    // Flags we will pass into SDL_SetVideoMode
    int flags = 0;
    //initialize SDL's video subsystem.
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        throw stringprintf("Video initialization failed: %s\n",
                           SDL_GetError( ) ).c_str();
    }
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_WM_GrabInput( SDL_GRAB_ON );
    SDL_EventState( SDL_VIDEOEXPOSE, SDL_DISABLE );
    SDL_EventState( SDL_SYSWMEVENT, SDL_DISABLE );
    SDL_EnableUNICODE( 1 );

    flags = SDL_OPENGL;
    // fullscreen and resizable exclude each other!
    if (fullscreen){
        flags |= SDL_FULLSCREEN;
    } else {
        flags |= SDL_RESIZABLE;
    }
    // set the video mode
    if( SDL_SetVideoMode( width, height, bpp, flags ) == 0 ) {
        throw stringprintf("Video mode set failed: %s\n",
                            SDL_GetError( ) ).c_str();
    }
    SDL_WM_SetCaption("kubix", NULL);
    SDL_Surface* icon = IMG_Load("./res/kubix.png");
    SDL_WM_SetIcon(icon, NULL);
    atexit(SDL_Quit);
}

/// initialize opengl
void initOpenGL(int width, int height){
    /* Our shading model--Gouraud (smooth). */
    glShadeModel( GL_SMOOTH );
    // use grey background (20%)...
    const KBX_Color& bgColor = KBX_Color::GREY10;
    glClearColor(  bgColor.r
                 , bgColor.g
                 , bgColor.b
                 , 0.0f);
    // ...and tell the object list that our background color
    // does not correspond to any kind of object
    KBX_Object::objectList.nullId = bgColor.id();
    setGLWindow(width, height);
    // use smooth shading model
    glShadeModel(GL_SMOOTH);
    // draw objects respecting depth
    glEnable(GL_DEPTH_TEST);
}


void setSDLWindow(int width, int height, bool resizable){
    int flags = 0;
    flags |= SDL_OPENGL;
    if(resizable) flags |= SDL_RESIZABLE;
    SDL_Surface* screen = SDL_SetVideoMode(width, height, 0, flags);
    if(!screen){
      throw stringprintf("Unable to open SDL Window!").c_str();
    }
}

void setGLWindow(int width, int height){
    GLfloat aspectRatio = (GLfloat)width / (GLfloat)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(10.0,        // the camera distance
                   aspectRatio, // the width-to-height ratio
                   1.0,         // the near z clipping coordinate
                   1024.0);     // the far z clipping coordinate
}  

GLuint glBuildFont(){
    Display *dpy;
    XFontStruct *fontInfo;  // storage for our font.
    GLuint base = glGenLists(96);                      // storage for 96 characters.
    // get the current display.  This opens a second
    // connection to the display in the DISPLAY environment
    // value, and will be around only long enough to load 
    // the font. 
    dpy = XOpenDisplay(NULL); // default to DISPLAY env.   
    fontInfo = XLoadQueryFont(dpy, "-adobe-helvetica-medium-r-normal--18-*-*-*-p-*-iso8859-1");
    if (fontInfo == NULL) {
	fontInfo = XLoadQueryFont(dpy, "fixed");
	if (fontInfo == NULL) {
            throw "no X font available!";
	}
    }
    // after loading this font info, this would probably be the time
    // to rotate, scale, or otherwise twink your fonts.  
    // start at character 32 (space), get 96 characters (a few characters past z), and
    // store them starting at base.
    glXUseXFont(fontInfo->fid, 32, 96, base);
    // free that font's info now that we've got the 
    // display lists.
    XFreeFont(dpy, fontInfo);
    // close down the 2nd display connection.
    XCloseDisplay(dpy);
    return base;
}

// delete the font.
GLvoid glKillFont( GLuint base ){
    // delete all 96 characters.
    glDeleteLists(base, 96);
}

GLvoid glPrint(const char *text, GLuint base){
    // if there's no text, do nothing.
    if (text == NULL) {                         
        return;
    }
    // alert that we're about to offset the display lists with glListBase
    glPushAttrib(GL_LIST_BIT);                  
    // sets the base character to 32.
    glListBase(base - 32);                      
    // draws the display list text.
    //    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); 
    glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); 
    // undoes the glPushAttrib(GL_LIST_BIT);
    glPopAttrib();                              
}
