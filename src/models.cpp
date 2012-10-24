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
      // this implementation of a (distinct) color id can
      // lead to pretty large numbers. it is used for the
      // picking mechanism. every object gets an own color
      // to distinguish it from the others when a click happens.
      // therefore, the size of the id-value depends highly
      // on the number of objects.
      // additionally, the picking-mechanism starts by 
      // assigning all shades of red before going over to the
      // other numbers. we therefore dont expect much trouble here...
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
  
  /// clear all object states
  /**
     set all objects isSelected, isMarked and isHighlighted state to false
  */
  void Scene::clearStates(){
    for(size_t id = 0; id < objList.size(); id++){
      objList[id]->clearStates();
    }
  }

  /// clear all tile states
  /**
     set all tiles isSelected, isMarked and isHighlighted state to false
  */
  void Board::clearStates(){
    for (size_t x=0; x < this->_nX; x++){
      for (size_t y=0; y < this->_nY; y++){
	this->_tiles[x][y]->clearStates();
      }
    }
  }

  /// clear all states
  /*
    set isSelected, isMarked and isHighlighted to false
  */
  void Object::loadTexture(QString filename, GLuint* textures, size_t nTexture){
    QImage tex, img = QImage(filename);
    tex = QGLWidget::convertToGLFormat( img );
    glBindTexture(GL_TEXTURE_2D, textures[nTexture]);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      tex.width(),
      tex.height(),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      tex.bits()
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  void Object::clearStates(){
    this->setSelectedState(false);
    this->setMarkedState(false);
    this->setHighlightedState(false);
  }
  
  // standard colors for marked, selected and highlighted states
  Color Object::cSelected = Color(0,0,255);
  Color Object::cMarked = Color(255,0,0);
  Color Object::cHighlighted = Color(0,255,255);

  /// default constructor
  Object::Object(Scene* scene) :
    _angle(0)
    ,_isVisible(true)
    ,_pos( Vec(0,0,0) )
    ,isHighlighted(false)
    ,isMarked(false)
    ,isSelected(false)
    ,scene(scene)
      // we need to add the object to the object list
      // and retrieve our own unique id from there
       //      ,id(Object::objectList.add(this)) 
  {}
  /// constructor setting the object's position
  Object::Object(Scene* scene, Vec pos) :
    _angle(0)
    ,_isVisible(true)
    ,_pos( pos )
    ,isHighlighted(false)
    ,isMarked(false)
    ,isSelected(false)
    ,scene(scene)
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
  void Object::setSelectedState(bool selected){
    this->isSelected = selected;
  }
  void Object::setMarkedState(bool marked){
    this->isMarked = marked;
  }
  void Object::setHighlightedState(bool highlighted){
    this->isHighlighted = highlighted;
  }
  Object* Object::clicked(size_t id){
    if(this->scene->idcount == id) return this;
    this->scene->idcount++;
    return NULL;
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
  void Object::display(){
      // save transformations done before
      glPushMatrix();
      this->_setColor();
      this->_translate();
      this->_rotate();
      this->_render();
      // reload transformations done before
      glPopMatrix();
  }
  void Object::_setColor(){
    if(!this->scene)
      throw "Cannot display object not belonging to any scene!";
    if(this->scene->picking){
      Color(this->scene->idcount).glColor();
      this->scene->idcount++;
    } else {
      this->setColor();
    }
  }
  
  /// set the object color before rendering
  void Object::setColor(){
      if(this->isSelected){
	this->cSelected.glColor();
	return;
      }
      if(this->isHighlighted){
	this->cHighlighted.glColor();
	return;
      }
      if(this->isMarked){
	this->cMarked.glColor();
	return;
      }
      Color::WHITE.glColor();
  }

  void Tile::setColor(){
      if(this->isSelected){
	this->cSelected.glColor();
	return;
      }
      if(this->isHighlighted){
	this->cHighlighted.glColor();
	return;
      }
      if(this->isMarked){
	this->cMarked.glColor();
	return;
      }
      this->basicColor.glColor();
  }
  
  /// inherit parent constructor
  AnimObject::AnimObject(Scene* scene) :
    Object(scene)
  {}
  /// inherit parent constructor
  AnimObject::AnimObject(Scene* scene, Vec pos) :
    Object(scene, pos)
  {}
  
  /// inherit parent constructor and set color
  Die::Die(Scene* scene, Vec pos, PlayColor color, GLuint* textures) :
    AnimObject(scene, pos)
    ,textures(textures)
    ,_playColor(color)
    ,IS_KING(false)
  {
    Die::loadTextures();
  }
  Die::Die(Scene* scene, Vec pos, PlayColor color, GLuint* textures, bool IS_KING) :
    AnimObject(scene, pos)
    ,_playColor(color)
    ,IS_KING(IS_KING)
  {
    Die::loadTextures();
  }
  /// render the die
  void Die::_render(){
    // setting the color is neccessary in order to ensure that the texture is drawn 'as-is'
    // leaving this out might cause the texture to be drawn with 'shaded' colors
    // because all texture-pixel rgb values are multiplied with the corresponding values 
    // of the current color before being drawn!
    glEnable( GL_TEXTURE_2D );
    // face 1
    if(this->scene->picking){
      glBindTexture( GL_TEXTURE_2D, this->textures[0] );
    }
    glBegin( GL_QUADS );
    glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,-0.5);
    glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,-0.5);
    glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,-0.5);
    glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,-0.5);
    glEnd();
    // face 2
    if(this->scene->picking){
      if ( ! this->IS_KING){
	glBindTexture( GL_TEXTURE_2D, this->textures[1] );
      }
      int b=0;
      if (this->_playColor == BLACK){
        b=1;
      }
      glEnable(GL_TEXTURE_2D);
      // face 1
      if(this->scene->picking){
        if ( this->IS_KING ){
          glBindTexture( GL_TEXTURE_2D, Die::textures[12+b] );
        } else {
          glBindTexture( GL_TEXTURE_2D, Die::textures[0+b] );
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
        if ( ! this->IS_KING){
          glBindTexture( GL_TEXTURE_2D, Die::textures[2+b] );
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
          glBindTexture( GL_TEXTURE_2D, Die::textures[4+b] );
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
          glBindTexture( GL_TEXTURE_2D, Die::textures[6+b] );
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
          glBindTexture( GL_TEXTURE_2D, Die::textures[8+b] );
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
          glBindTexture( GL_TEXTURE_2D, Die::textures[10+b] );
        }
      }
      glBegin( GL_QUADS );
       glTexCoord2f(0.0,1.0); glVertex3f(-0.5,-0.5,+0.5);
       glTexCoord2f(1.0,1.0); glVertex3f(+0.5,-0.5,+0.5);
       glTexCoord2f(1.0,0.0); glVertex3f(+0.5,+0.5,+0.5);
       glTexCoord2f(0.0,0.0); glVertex3f(-0.5,+0.5,+0.5);
      glEnd();
      glDisable(GL_TEXTURE_2D);
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
	this->_tiles[x][y] = new Tile(this->scene, tilePosition, tileColor );
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
  void Board::_render(){
      for (size_t x=0; x < this->_nX; x++){
          for (size_t y=0; y < this->_nY; y++){
              glLoadName(x+y*this->_nY);
              this->_tiles[x][y]->display();
          }
      }
  }
  /// return gui id of tile defined by its coordinates
  // TODO: this piece of code is deprecated and should be replaced by something else
  //size_t Board::getTileId(size_t x, size_t y){
  //    return this->_tiles[x][y]->id;
  //}
  
  /// tile constructor
  Tile::Tile(Scene* scene, Vec pos, Color color) :
    Object(scene, pos)
    ,basicColor(color)
  {
  }
  /// render the tile
  void Tile::_render(){
      Logger log("Tile::_render");
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
    :Object(this)
    ,picking(false)
    ,act(act)
    ,cam(Vec(0,0,100),Vec(0,0,0))
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
    this->_dice.push_back( new Die(this, Vec(-4,0, 4), WHITE ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-3,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-2,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec(-1,0, 4), WHITE ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 0,0, 4), WHITE, true ) );
    this->_dice.push_back( new Die( Vec( 1,0, 4), WHITE ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 2,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 3,0, 4), WHITE ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( counterClockwise, 90 );
    this->_dice.push_back( new Die( Vec( 4,0, 4), WHITE ) );
    this->_dice.back()->rotate( counterClockwise, 90 );
    // black dice; b1 is in upper left corner, b8 in upper right
    this->_dice.push_back( new Die( Vec(-4,0,-4), BLACK ) );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-3,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-2,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec(-1,0,-4), BLACK ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 0,0,-4), BLACK, true ) );
    this->_dice.push_back( new Die( Vec( 1,0,-4), BLACK ) );
    this->_dice.back()->rotate( toFront, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 2,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 180 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 3,0,-4), BLACK ) );
    this->_dice.back()->rotate( toBack, 90 );
    this->_dice.back()->rotate( clockwise, 90 );
    this->_dice.push_back( new Die( Vec( 4,0,-4), BLACK ) );
    this->_dice.back()->rotate( clockwise, 90 );
    // add dice to scene
    for (size_t i=0; i < this->_dice.size(); i++){
        // add dice to scene
        this->add( this->_dice[i] );
        // add mapping from id of gui-die to
        // internal id (here: i) of abstract representation
	//        this->_id2Die[ this->_dice[i]->id ] = i;
    }
    // initialize the board and add it to the scene
    this->_board = new Board(this, 9, 9);
    this->add( this->_board );
    // add mapping from id of board-tile (gui representation) to
    // its xy-coordinates
    /*
      TODO: this piece of code is deprecated and should be replaced by something else
    for (size_t x=0; x < 9; x++){
        for (size_t y=0; y < 9; y++){
            std::pair<size_t,size_t> xy(x,y);
	    //            this->_id2Field[ this->_board->getTileId(x,y) ] = xy;
        }
    }*/
  }
  
  // scene destructor
  Scene::~Scene() {}
   
  /// render the scene
  void Scene::_render(){
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
      // TODO: test if the following line can (maybe?) be deleted
      glLoadName(i);
      this->objList[i]->display();
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


/// pick Object at given mouse coordinates
  /**
      \param p mouse coordinates
      \returns Object* to object under mouse cursor
  */
  Object* Scene::pickObject(QPoint p){
    // get resolution from settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // get color information from frame buffer
    unsigned char pixel[3];
    this->idcount = 0;
    this->picking = true;
    glClearColor(1.0f,1.0f,1.0f,0.0f);
    this->display();
    // Important: gl (0,0) is bottom left but window coords (0,0) are top left -> have to subtract y from height
    glReadPixels(p.x(), viewport[3] - p.y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    size_t id = Color(pixel[0], pixel[1], pixel[2]).id();
    this->idcount = 0;
    picking = false;
    glClearColor(  this->act->bgColor.r, this->act->bgColor.g, this->act->bgColor.b, 0.0f);
    Logger l("Scene::pickObject");
    l.info(stringprintf("clicked object id: %d",(int)id));
    Object* obj = this->clicked(id);
    if(obj) return obj;
    l.warning(stringprintf("sorry, could not find object matching id %d",(int)id));
    return NULL;
  }
  Object* Scene::clicked(size_t id){
    for(size_t i=0; i<this->objList.size(); i++){
      Object* obj = this->objList[i]->clicked(id);
      if(obj) return obj;
    }
  }
  Object* Board::clicked(size_t id){
    Object* obj;
    for (size_t x=0; x < this->_nX; x++){
      for (size_t y=0; y < this->_nY; y++){
	obj = this->_tiles[x][y]->clicked(id);
	if(obj) return obj;
      }
    }
    return NULL;
  }
  /// mark next object on the board
  /**
      \param dx relative coordinate in x direction depending on currently marked object
      \param dy relative coordinate in y direction depending on currently marked object
      
      mark next object on relative coordinates depending on the currently marked object.
      if there is no object marked, automatically mark the field in the middle.
  */
  void Scene::markNext(int dx, int dy){
    // TODO: This piece of code is deprecated and should be replaced with something else
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
  
} // end namespace KBX
