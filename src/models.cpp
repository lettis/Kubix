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

#include <GL/glu.h>
//#include <GL/glx.h>

#include "models.hpp"
#include "gui_opengl.hpp"
#include "tools.hpp"
// engine is needed for PlayColor definition
#include "engine.hpp"


namespace KBX {
  // define fixed color values
  const Color Color::BLACK (0.0f, 0.0f, 0.0f);
  const Color Color::GREY10(0.1f, 0.1f, 0.1f);
  const Color Color::GREY20(0.2f, 0.2f, 0.2f);
  const Color Color::GREY30(0.3f, 0.3f, 0.3f);
  const Color Color::GREY40(0.4f, 0.4f, 0.4f);
  const Color Color::GREY50(0.5f, 0.5f, 0.5f);
  const Color Color::GREY60(0.6f, 0.6f, 0.6f);
  const Color Color::GREY70(0.7f, 0.7f, 0.7f);
  const Color Color::GREY80(0.8f, 0.8f, 0.8f);
  const Color Color::GREY90(0.9f, 0.9f, 0.9f);
  const Color Color::WHITE (1.0f, 1.0f, 1.0f);
  const Color Color::RED   (1.0f, 0.0f, 0.0f);
  const Color Color::GREEN (0.0f, 1.0f, 0.0f);
  const Color Color::BLUE  (0.0f, 0.0f, 1.0f);
  // define different versions of the Color constructor depending on parameters
  Color::Color(){
      this->r = 0;
      this->g = 0;
      this->b = 0;
  }
  Color::Color(size_t id){
      this->r = float(id%255)/255;
      this->g = float((id/255)%255)/255;
      this->b = float((id/(255*255))%255)/255;
  }
  Color::Color(unsigned char r, unsigned char g, unsigned char b){
      this->r = float(r)/255;
      this->g = float(g)/255;
      this->b = float(b)/255;
  }
  Color::Color(int r, int g, int b){
      this->r = float(r)/255;
      this->g = float(g)/255;
      this->b = float(b)/255;
  }
  Color::Color(float r, float g, float b){
      this->r = r;
      this->g = g;
      this->b = b;
  }
  size_t Color::id() const {
      // TODO: think about this. this implementation leads to some pretty large ids
      //       if the color is bright (e.g. white: 1.0*255 + 1.0*255^2 + 1.0*255^3 ).
      //       is that necessary?
      return this->r*255 + 255*255*this->g + 255*255*255*this->b;
  }
  /// call glColor3f with internal values
  void Color::glColor() const {
      glColor3f( this->r, this->g, this->b );
  }
  
  /// constructor initializing nullvector
  Vec::Vec(){
      this->x = 0;
      this->y = 0;
      this->z = 0;
  }
  /// constructor of the Vec class 
  /**
     \param x x-corrdinate (left-right in standard view)
     \param y y-coordinate (up-down in standard view)
     \param z z-coordinate (front-back in standard view)
   */
  Vec::Vec(float x, float y, float z){
      this->x = x;
      this->y = y;
      this->z = z;
  }
  /// calculate the euclidian norm
  float Vec::norm(){
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
  Vec Vec::normalize(){
      float norm = this->norm();
      if (norm != 0){
          return Vec(   this->x/norm
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
  Vec Vec::scale(float a){
      return Vec( a*this->x, a*this->y, a*this->z );
  }
  /// add vector
  /**
      \param v the vector to be added
  */
  Vec Vec::add(Vec v){
      return Vec( this->x + v.x
  		           ,this->y + v.y
  		           ,this->z + v.z
  		   );
  }
  /// subtract vector
  /**
      \param v the vector to be subtracted
  */
  Vec Vec::sub(Vec v){
      return this->add( v.scale(-1) );
  }
  /// rotate vector around given axis
  /**
      \param rotAxis the axis around which the vector is to be rotated
      \param angle angle of rotation, given in degrees
      \returns the rotated vector
  */
  Vec Vec::rotate(Vec rotAxis, float angle){
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
      return Vec(x,y,z);
  }
  /// calculate the cross product
  /**
      \param v the other vector
      \returns the cross product  (this X v)
  */
  Vec Vec::cross(Vec v){
      Vec result;
      result.x = this->y*v.z - this->z*v.y;
      result.y = this->z*v.x - this->x*v.z;
      result.z = this->x*v.y - this->y*v.x;
      return result;
  }
  /// constructor initializing to pos=(0,0,0) and target=(0,0,-1)
  Camera::Camera(){
      this->position = Vec(0,0,0);
      this->target = Vec(0,0,-1);
  }
  /// constructor initializing to given pos and target=(0,0,-1)
  Camera::Camera(Vec pos){
      this->position = pos;
      this->target = Vec(0,0,-1);
  }
  /// constructor initializing to given pos and target
  Camera::Camera(Vec pos, Vec target){
      this->position = pos;
      this->target = target;
  }
  /// update the OpenGL camera perspective
  void Camera::updateView(){
      // set OpenGL camera
      gluLookAt( this->position.x, this->position.y, this->position.z,
                 this->target.x  , this->target.y  , this->target.z,
                 0,1,0
      );
  }
  /// get orientation of camera
  Vec Camera::getOrientation(){
    return this->position.sub( this->target );
  }
  
  /// set new target
  /**
      \param target the target to be set (i.e. the point where to look at)
  */
  void Camera::setTarget(Vec target){
      this->target = target;
  }
  /// set new position
  /**
      \param position the new position of the camera
  */
  void Camera::setPosition(Vec position){
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
  void Camera::rotate(float angle, size_t direction){
      Vec v=this->position.sub( this->target );
      if (direction == this->HORIZONTAL){
          // rotate in horizontal plane, i.e. around the y-axis
          v = v.rotate( Vec(0,1,0), angle );
          this->position = this->target.add( v );
      }else if (direction == this->VERTICAL){
          // rotate in vertical plane, i.e. around the axis
          // orthogonal to the y-axis and the vector v.
          if (   (v.normalize().y < 0.99 && angle > 0)  
               ||(v.normalize().y > -0.99 && angle < 0) ){
              Vec ortho = v.cross( Vec(0,1,0) );
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
  void Camera::zoom(float factor){
      if (factor <= 0){
          throw "cannot zoom by negative or zero zoom factor";
      }
      Vec diff = this->position.sub( this->target );
      diff = diff.scale( factor );
      if(diff.norm() < 900 && diff.norm() > 15){
          this->position = this->target.add( diff );
      }
  }
  
  /// get object with given id
  /**
     \param id id of the desired object
     \return Object with given id
  */
  Object* ObjectHandler::get(size_t id){
      Logger log("ObjectHandler::get");
      // TODO: calculation of ids seems to cause problems
      //       for certain values, this must be changed.
      //       example: choose GREY10 as background color
      //                instead of GREY20
      log.info( stringprintf( "   null-id:\t%d", this->nullId ) );
      log.info( stringprintf( "clicked id:\t%d", id) );
      if ( id == this->nullId ){
          return NULL;
      } else if(id > this->objects.size()){
          throw stringprintf("cannot access object %d: not in list", int(id)).c_str();
      } else {
          return this->objects.at(id);
      }
  }
  /// construct a new object handler
  ObjectHandler::ObjectHandler(){
      this->nullId = 0;
  }
  /// add an object to the handler
  /**
     \param obj object to add to the handler
     \return number of object in list (object id)
  */
  size_t ObjectHandler::add(Object* obj){
      if(this->objects.size() == nullId){
          this->objects.push_back(NULL);
      }
      this->objects.push_back(obj);
      return this->objects.size()-1;
  }
  /// remove an object from the handler
  /**
     \param obj object to remove from the handler
   */
  void ObjectHandler::remove(Object* obj){
      for(size_t i=0; i<this->objects.size(); i++){
          if(objects[i] == obj) objects[i] = NULL;
      }
  }
  /// remove an object from the handler
  /**
     \param id id of object to remove from the handler
   */
  void ObjectHandler::remove(size_t id){
      if(id < objects.size()){
          objects[id] = NULL;
      }
  }
  
  /// list of all objects to be handled (mouse events)
  ObjectHandler Object::objectList;
  /// default constructor
  Object::Object() :
       _angle(0)
      ,_isVisible(true)
      ,_pos( Vec(0,0,0) )
      ,activityState(DEFAULT)
      // we need to add the object to the object list
      // and retrieve our own unique id from there
      ,id(Object::objectList.add(this)) 
  {}
  /// constructor setting the object's position
  Object::Object(Vec pos) :
       _angle(0)
      ,_isVisible(true)
      ,_pos( pos )
      ,activityState(DEFAULT)
      // we need to add the object to the object list
      // and retrieve our own unique id from there
      ,id(Object::objectList.add(this)) 
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
  void Object::rotate(Vec axis, float angle){
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
  void Object::translate(Vec direction){
      this->_pos = direction;
  }
  /// actually rotate object (private, only called by 'display')
  void Object::_rotate(){
      for(size_t i=0; i<this->_angle.size(); i++){
          glRotatef( this->_angle[i], this->_rotAxis[i].x, this->_rotAxis[i].y, this->_rotAxis[i].z );
      }
  }
  /// actually translate object (private, only called by 'display')
  void Object::_translate(){
      glTranslatef( this->_pos.x, this->_pos.y, this->_pos.z );
  }
  /// display the object
  void Object::display(bool picking){
      // save transformations done before
      glPushMatrix();
      this->_translate();
      this->_rotate();
      this->_render(picking);
      // reload transformations done before
      glPopMatrix();
  }
  
  /// inherit parent constructor
  AnimObject::AnimObject() :
      Object()
  {}
  /// inherit parent constructor
  AnimObject::AnimObject(Vec pos) :
      Object(pos)
  {}
  
  //TODO: weg
  // this defines the keys for the die face textures
  //const size_t Die::FACE_K_W = 10;
  //const size_t Die::FACE_K_B = 20;
  //const size_t Die::FACE_1_W = 11;
  //const size_t Die::FACE_2_W = 12;
  //const size_t Die::FACE_3_W = 13;
  //const size_t Die::FACE_4_W = 14;
  //const size_t Die::FACE_5_W = 15;
  //const size_t Die::FACE_6_W = 16;
  //const size_t Die::FACE_1_B = 21;
  //const size_t Die::FACE_2_B = 22;
  //const size_t Die::FACE_3_B = 23;
  //const size_t Die::FACE_4_B = 24;
  //const size_t Die::FACE_5_B = 25;
  //const size_t Die::FACE_6_B = 26;

  //TODO: weg
  /// the textures of the die surfaces are handled "globally" by this static member
  //TextureHandler Die::textures = TextureHandler();

  /// inherit parent constructor and set color
  Die::Die(Vec pos, PlayColor color, GLuint* textures) :
       AnimObject(pos)
      ,textures(textures)
      ,_playColor(color)
      ,IS_KING(false)
  {
      this->setColors();
  }
  Die::Die(Vec pos, PlayColor color, GLuint* textures, bool IS_KING) :
       AnimObject(pos)
      ,textures(textures)
      ,_playColor(color)
      ,IS_KING(IS_KING)
  {
      this->setColors();
  }
  /// set colors for different activity states
  void Die::setColors(){
      this->coloring[DEFAULT]     = Color::WHITE;
      this->coloring[HIGHLIGHTED] = Color::GREEN;
      this->coloring[MARKED]      = Color::BLUE;
      this->coloring[SELECTED]    = Color::RED;
  }
  /// render the die
  void Die::_render(bool picking){
      // setting the color is neccessary in order to ensure that the texture is drawn 'as-is'
      // leaving this out might cause the texture to be drawn with 'shaded' colors
      // because all texture-pixel rgb values are multiplied with the corresponding values 
      // of the current color before being drawn!
      if(picking){
          Color pickerColor = Color(this->id);
          pickerColor.glColor();
      } else {
          switch(this->activityState) {
              case DEFAULT:
                  this->coloring[DEFAULT].glColor();
                  break;
              case HIGHLIGHTED:
                  this->coloring[HIGHLIGHTED].glColor();
                  break;
              case SELECTED:
                  this->coloring[SELECTED].glColor();
                  break;
              case MARKED:
                  this->coloring[MARKED].glColor();
                  break;
              default:
                  this->coloring[DEFAULT].glColor();
                  break;
          }
          glEnable( GL_TEXTURE_2D );
      }
      // face 1
      if(!picking){
        glBindTexture( GL_TEXTURE_2D, this->textures[0] );
      }
      glBegin( GL_QUADS );
       glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
       glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
       glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,-0.5);
       glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,-0.5);
      glEnd();
      // face 2
      if(!picking){
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, this->textures[1] );
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
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, this->textures[2] );
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
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, this->textures[3] );
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
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, this->textures[4] );
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
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, this->textures[5] );
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
  Board::Board(size_t nX, size_t nY) :
       _nX(nX)
      ,_nY(nY)
      ,_tiles(_nX, std::vector<Tile*>(_nY))
  {
      // define tile colors
      Color dark = Color::GREY40;
      Color bright = Color::GREY60;
      Color tileColor;
      Vec tilePosition;
      // setup tiles to form a checkered layout
      for(size_t x=0; x < this->_nX; x++){
          for(size_t y=0; y < this->_nY; y++){
              tileColor = ( (x%2 + y%2)%2 == 0 ) ? dark : bright;
              tilePosition = Vec(   (float)x - (float)(this->_nX)/2
                                       ,-0.5
                                       ,-(float)y + (float)(this->_nY)/2 -1
                             );
              this->_tiles[x][y] = new Tile( tilePosition, tileColor );
          }
      }
  }
  /// free memory of allocated tiles in destructor
  Board::~Board(){
      std::vector<Tile*>::iterator ys;
      for (size_t x=0; x < this->_nX; x++){
          for (ys=this->_tiles[x].begin(); ys < this->_tiles[x].end(); ys++){
              delete *ys;
          }
      }
  }
  /// display board by rendering every tile
  void Board::_render(bool picking){
      for (size_t x=0; x < this->_nX; x++){
          for (size_t y=0; y < this->_nY; y++){
              glLoadName(x+y*this->_nY);
              this->_tiles[x][y]->display(picking);
          }
      }
  }
  /// return gui id of tile defined by its coordinates
  size_t Board::getTileId(size_t x, size_t y){
      return this->_tiles[x][y]->id;
  }
  
  /// tile constructor
  Tile::Tile(Vec pos, Color color) :
       Object(pos)
      ,basicColor(color)
  {
      this->setColors();
  }
  /// set colors for different activity states
  void Tile::setColors(){
      this->coloring[DEFAULT]     = this->basicColor;
      this->coloring[HIGHLIGHTED] = Color::GREEN;
      this->coloring[MARKED]      = Color::BLUE;
      this->coloring[SELECTED]    = Color::RED;
  }
  /// render the tile
  void Tile::_render(bool picking){
      Logger log("Tile::_render");
      if(picking){
          Color(this->id).glColor();
      } else {
          switch(this->activityState) {
              case DEFAULT:
                  this->coloring[DEFAULT].glColor();
                  break;
              case HIGHLIGHTED:
                  this->coloring[HIGHLIGHTED].glColor();
                  break;
              case SELECTED:
                  this->coloring[SELECTED].glColor();
                  break;
              case MARKED:
                  this->coloring[MARKED].glColor();
                  break;
              default:
                  this->coloring[DEFAULT].glColor();
                  break;
          }
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
  
  /// scene constructor
  Scene::Scene(GLWidget* act)
    :act(act),
     cam(Vec(0,0,100),Vec(0,0,0))
  {
    // define rotation axes
    // TODO: x/y coordinates of gui and engine seem not to match
    Vec toFront         (-1.0,  0.0,  0.0);
    Vec toBack          ( 1.0,  0.0,  0.0);
    Vec toLeft          ( 0.0,  0.0,  1.0);
    Vec toRight         ( 0.0,  0.0, -1.0);
    Vec clockwise       ( 0.0, -1.0,  0.0);
    Vec counterClockwise( 0.0,  1.0,  0.0);

    // setup dice with correct orientation.
    // per definition, the dice are set up in the same order as the dice
    // are defined in the engine.
    // this way, the id of a single die in the _dice vector and in the engine
    // are the same. from this, the following order follows:
    //      white dice, from left to right (sight from behind white) have ids
    //      0, 1, 2, 3, 4 (king), 5, 6, 7, 8
    //      black dice, from left to right (sight from behind white) have ids
    //      9, 10, 11, 12, 13 (king), 14, 15, 16, 17.
  
    // white dice; w1 is in lower left corner, w8 in lower right
    this->_dice.push_back( new Die( Vec(-4,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-3,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-2,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-1,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 0,0, 4), WHITE, this->act->textures->kingTextureWhite, true ) );
    this->_dice.push_back( new Die( Vec( 1,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 2,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 3,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 4,0, 4), WHITE, this->act->textures->dieTexturesWhite ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    // black dice; b1 is in upper left corner, b8 in upper right
    this->_dice.push_back( new Die( Vec(-4,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-3,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-2,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-1,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 0,0,-4), BLACK, this->act->textures->kingTextureBlack, true ) );
    this->_dice.push_back( new Die( Vec( 1,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 2,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 3,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 4,0,-4), BLACK, this->act->textures->dieTexturesBlack ) );
    this->_dice.back()->rotate( clockwise, 90 );
    // add dice to scene
    for (size_t i=0; i < this->_dice.size(); i++){
        // add dice to scene
        this->add( this->_dice[i] );
        // add mapping from id of gui-die to
        // internal id (here: i) of abstract representation
        this->_id2Die[ this->_dice[i]->id ] = i;
    }
    // initialize the board and add it to the scene
    this->_board = new Board(9, 9);
    this->add( this->_board );
    // add mapping from id of board-tile (gui representation) to
    // its xy-coordinates
    for (size_t x=0; x < 9; x++){
        for (size_t y=0; y < 9; y++){
            std::pair<size_t,size_t> xy(x,y);
            this->_id2Field[ this->_board->getTileId(x,y) ] = xy;
        }
    }
  }
  
  // scene destructor
  Scene::~Scene() {}
  
  /// render the scene
  void Scene::_render(bool picking){
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
      // draw everything to screen
      if(!picking){
          //TODO: SwapBuffers
      }
  }
  /// add object to the scene
  /**
      \param obj pointer to a Object to be added to the scene
      \throws string exception, if trying to add null-reference to scene
  */
  void Scene::add(Object* obj){
      if (obj){
          this->objList.push_back( obj );
      }else{
          throw "unable to add object to scene (null-reference)";
      }
  }
  /// rotate the scene
  /**
      \param angle the angle around which the scene should be rotated
      \param direction the direction of the rotation, either Camera.HORIZONTAL or Camera.VERTICAL
  */
  void Scene::rotate(float angle, size_t direction){
      this->cam.rotate( angle, direction );
  }
  
  /// obtain camera orientation relative to center of scene
  Vec Scene::getOrientation(){
      return this->cam.getOrientation();
  }
  
  /// zoom the scene in/out
  /**
      \param factor the zoom factor
  */
  void Scene::zoom(float factor){
      this->cam.zoom( factor );
  }


  /// mark next object on the board
  /**
      \param dx relative coordinate in x direction depending on currently marked object
      \param dy relative coordinate in y direction depending on currently marked object
      
      mark next object on relative coordinates depending on the currently marked object.
      if there is no object marked, automatically mark the field in the middle.
  */
  void Scene::markNext(int dx, int dy){
//    int x,y;
//    if (this->_markedId == CLEAR){
//      // if no field marked, set centre field marked
//      x = 3;
//      y = 2;
//    } else {
//      if ( this->_id2Field.count( this->_markedId ) == 1 ){
//        // prev. marked obj. was a field
//        std::pair<size_t,size_t> xy = this->_id2Field[ this->_markedId ];
//        x = xy.first;
//        y = xy.second;
//      } else {
//        // prev. marked obj. was a die
//        size_t dieId = this->_id2Die[ this->_markedId ];
//        DieState* die = this->_game->getDie( dieId );
//        x = die->x();
//        y = die->y();
//      }
//      // check boundaries
//      if ( 0 <= x+dx && x+dx < 9 ){
//        x += dx;
//      }
//      if ( 0 <= y+dy && y+dy < 9 ){
//        y += dy;
//      }
//    }
//    size_t id = this->_board->getTileId( x, y );
//    // mark the object
//    //TODO: bullshit
//    //this->_mark( id );
  }







//TODO: deprecated
/*
  /// initialize opengl
  void initOpenGL(int width, int height){
      // Our shading model--Gouraud (smooth).
      glShadeModel( GL_SMOOTH );
      // use grey background (10%)...
      const Color& bgColor = Color::GREY20;
      glClearColor(  bgColor.r
                   , bgColor.g
                   , bgColor.b
                   , 0.0f);
      // ...and tell the object list that our background color
      // does not correspond to any kind of object
      Object::objectList.nullId = bgColor.id();
      setGLWindow(width, height);
      // use smooth shading model
      glShadeModel(GL_SMOOTH);
      // draw objects respecting depth
      glEnable(GL_DEPTH_TEST);
  }
*/
  
//TODO: deprecated
/*
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
*/
  
} // end namespace KBX
