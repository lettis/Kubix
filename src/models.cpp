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
#include <cassert>
#include <map>

#include <GL/glu.h>

#include "models.hpp"
#include "game_widget.hpp"
#include "tools.hpp"
#include "engine.hpp"

namespace KBX {
  /// constructor initializing to pos=(0,0,0) and target=(0,1,0)
  Camera::Camera() {
    this->_position = Vec(0, 0, 0);
    this->_target = Vec(0, 1, 0);
  }

  /// constructor initializing to given pos and target=(0,1,0)
  Camera::Camera(Vec pos) {
    this->_position = pos;
    this->_target = Vec(0, 1, 0);
  }

  /// constructor initializing to given pos and target
  Camera::Camera(Vec pos, Vec target) {
    this->_position = pos;
    this->_target = target;
  }

  /// update the OpenGL camera perspective
  void Camera::updateView() {
    // set OpenGL camera
    gluLookAt(this->_position.x, this->_position.y, this->_position.z, this->_target.x, this->_target.y, this->_target.z,
	      0, 0, 1);
  }

  /// get orientation of camera
  Vec Camera::getOrientation() {
    return this->_position.sub(this->_target);
  }

  /// set new target
  /**
     \param target the target to be set (i.e. the point where to look at)
  */
  void Camera::setTarget(Vec target) {
    this->_target = target;
  }

  /// set new position
  /**
     \param position the new position of the camera
  */
  void Camera::setPosition(Vec position) {
    this->_position = position;
  }

  /// set new camera position by rotating around target
  /**
     \param angle angle of rotation [degrees]
     \param direction rotate horizontally or vertically
     angle may be positive or negative.
     positive means to the right (horizontal) or upwards (vertical).
     negative means to the left (horizontal) or downwards (vertical).
  */
  void Camera::rotate(float angle, size_t direction) {
    Vec v = this->_position.sub(this->_target);
    if (direction == this->HORIZONTAL) {
      // rotate in horizontal plane, i.e. around the y-axis
      v = v.rotate(Vec(0, 0, 1), angle);
      this->_position = this->_target.add(v);
    } else if (direction == this->VERTICAL) {
      // rotate in vertical plane, i.e. around the axis
      // orthogonal to the y-axis and the vector v.
      if ((v.normalized().z < 0.99 && angle > 0) || (v.normalized().z > -0.99 && angle < 0)) {
	Vec ortho = v.cross(Vec(0, 0, 1));
	v = v.rotate(ortho, angle);
	this->_position = this->_target.add(v);
      }
    } else {
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
  void Camera::zoom(float factor) {
    if (factor <= 0) {
      throw "cannot zoom by negative or zero zoom factor";
    }
    Vec diff = this->_position.sub(this->_target);
    diff = diff.scaled(factor);
    if (diff.norm() < 900 && diff.norm() > 15) {
      this->_position = this->_target.add(diff);
    }
  }

  /// load Object textures
  /*
    load texture for die surface
  */
  void Model::loadTexture(QString filename, GLuint* textures, size_t nTexture) {
    QImage tex, img = QImage(filename);
    tex = QGLWidget::convertToGLFormat(img);
    glBindTexture(GL_TEXTURE_2D, textures[nTexture]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  Vec Model::getPrimaryOrientation(){
    return this->_primaryOrientation;
  }
  Vec Model::getSecondaryOrientation(){
    return this->_secondaryOrientation;
  }

  /// clear all states
  /*
    set isSelected, isMarked and isHighlighted to false
  */
  void Model::clearStates() {
    this->setSelectedState(false);
    this->setMarkedState(false);
    this->setHighlightedState(false);
  }

  // standard colors for marked, selected and highlighted states
  Color Model::cSelected = Color(0, 0, 255);
  Color Model::cMarked = Color(255, 0, 0);
  Color Model::cHighlighted = Color(0, 255, 255);

  /// default constructor
  Model::Model(Scene* scene)
    : _id((scene && scene != this) ? scene->newId() : -1),
      _primaryOrientation(NormalVectors::X),
      _secondaryOrientation(NormalVectors::Y),
      _pos(Vec(0, 0, 0)),
      _isVisible(true),
      _isHighlighted(false),
      _isMarked(false),
      _isSelected(false),
      _scene(scene)
      // we need to add the object to the object list
      // and retrieve our own unique id from there
      //      ,id(Object::objectList.add(this))
  {
  }

  /// constructor setting the object's position
  Model::Model(Scene* scene, Vec pos)
    : _id((scene && scene != this) ? scene->newId() : -1),
      _primaryOrientation(NormalVectors::X),
      _secondaryOrientation(NormalVectors::Y),
      _pos(pos),
      _isVisible(true),
      _isHighlighted(false),
      _isMarked(false),
      _isSelected(false),
      _scene(scene) {
  }

  /// default destructor
  Model::~Model() {
  }

  Scene* Model::getScene(){
    return this->_scene;
  }

  void Model::setOrientation(Vec primary, Vec secondary) {
    this->_primaryOrientation = primary.normalized();
    this->_secondaryOrientation = secondary.normalized();
  }

  void Model::rotate(Vec axis, float angle) {
    this->_primaryOrientation = this->_primaryOrientation.rotate(axis, angle);
    this->_secondaryOrientation = this->_secondaryOrientation.rotate(axis, angle);
  }
  void Model::setSelectedState(bool selected) {
    this->_isSelected = selected;
  }
  void Model::setVisibleState(bool visible) {
    this->_isVisible = visible;
  }
  void Model::setMarkedState(bool marked) {
    this->_isMarked = marked;
  }
  void Model::setHighlightedState(bool highlighted) {
    this->_isHighlighted = highlighted;
  }
  Model* Model::clicked(size_t id) {
    if (this->_id.id() == id) {
      return this;
    } else {
      return NULL;
    }
  }

  /// set object translation
  void Model::translate(Vec direction) {
    this->_pos = direction;
  }
  /// actually rotate object (private, only called by 'display' or animation code)
  void Model::_rotate() {
    Vec v1 = this->_primaryOrientation;
    Vec v2 = this->_secondaryOrientation;
    Vec v3 = v1.cross(v2);
    GLfloat r[16] = {v1.x, v2.x, v3.x, 0.0f, v1.y, v2.y, v3.y, 0.0f, v1.z, v2.z, v3.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    glMultMatrixf(r);
  }
  /// actually translate object (private, only called by 'display')
  void Model::_translate() {
    glTranslatef(this->_pos.x, this->_pos.y, this->_pos.z);
  }
  /// display the object
  void Model::display() {
    if(!this->_isVisible) return;
    // save transformations done before
    glPushMatrix();
    this->_setRenderingColor();
    this->_translate();
    this->_rotate();
    this->_render();
    // reload transformations done before
    glPopMatrix();
  }
  void Model::_setRenderingColor() {
    if ( !this->_scene)
      throw "Cannot display object not belonging to any scene!";
    if (this->_scene->inObjPickingMode) {
      // render object with unique color mode for object picking
      // (click on obj -> click on unique color -> identify object)
      this->_id.setAsGlColor();
    } else {
      // render object with true color (for view)
      this->_setColor();
    }
  }

  /// set the object color before rendering
  void Model::_setColor() {
    if (this->_isSelected) {
      this->cSelected.setAsGlColor();
      return;
    }
    if (this->_isHighlighted) {
      this->cHighlighted.setAsGlColor();
      return;
    }
    if (this->_isMarked) {
      this->cMarked.setAsGlColor();
      return;
    }
    ColorTable::WHITE.setAsGlColor();
  }

  /// get position of object
  Vec Model::getPosition() {
    return this->_pos;
  }

  /// inherit parent constructor
  //AnimatedModel::AnimatedModel(Scene* scene)
  //    : _angleOrig(0),
  //      _rotStep(0),
  //      _transStep(0),
  //      _angleDest(0),
  //      Model(scene) {
  //}
  /// inherit parent constructor
  //AnimatedModel::AnimatedModel(Scene* scene, Vec pos)
  //    : _angleOrig(0),
  //      _rotStep(0),
  //      _transStep(0),
  //      _angleDest(0),
  //      Model(scene, pos) {
  //}

  // set texture loading flag to false initially
  bool Die::texturesLoaded = false;
  GLuint Die::textures[14];

  /// inherit parent constructor and set dieId (same as in engine)
  Die::Die(Scene* scene, Vec pos, size_t dieId)
    : Model(scene, pos),
      IS_KING((dieId == 4) || (dieId == 13)),
      _tile(NULL),
      _dieId(dieId),
      _isMoving(false){
    Die::loadTextures();
    if (this->_dieId < 9) {
      this->_playColor = WHITE;
    } else if (this->_dieId < 18) {
      this->_playColor = BLACK;
    } else {
      throw "die index must be between 0 and 17.";
    }
  }
    
  Die::Die(Scene* scene, size_t dieId)
    : Model(scene, Vec(0,0,0)),
      IS_KING((dieId == 4) || (dieId == 13)),
      _tile(NULL),
      _dieId(dieId),
      _isMoving(false){
    Die::loadTextures();
    if (this->_dieId < 9) {
      this->_playColor = WHITE;
    } else if (this->_dieId < 18) {
      this->_playColor = BLACK;
    } else {
      throw "die index must be between 0 and 17.";
    }
  }
  
  /// return die index
  size_t Die::getId() {
    return this->_dieId;
  }

  PlayColor Die::getPlayColor() {
    return this->_playColor;
  }

  /**
     load textures for models
  */
  void Die::loadTextures() {
    if ( !Die::texturesLoaded) {
      glGenTextures(14, Die::textures);
      for (size_t i = 1; i < 7; i++) {
        Die::loadTexture(QString(":res/side%1.png").arg(i), Die::textures, 2 * (i - 1));
        Die::loadTexture(QString(":res/side%1b.png").arg(i), Die::textures, 2 * (i - 1) + 1);
      }
      Die::loadTexture(QString(":res/sidek.png"), Die::textures, 12);
      Die::loadTexture(QString(":res/sidekb.png"), Die::textures, 13);
      Die::texturesLoaded = true;
    }
  }

  void Die::dissociate(){
    this->_tile->setDie(NULL);
  }

  void Die::reassociate(){
    this->_tile->setDie(this);
  }

  /// set the tile belonging to this die
  /**
     \param t new tile to be associated to this die
  */
  void Die::setTile(Tile* t) {
    if (this->_tile) {
      if (this->_tile->getDie() != this) {
        throw "trying to dissociate die from unassociated tile!";
      }
      this->_tile->setDie(NULL);
    }
    this->_tile = t;
    if (this->_tile) {
      if (this->_tile->getDie() != NULL) {
        throw "trying to associate die to already occupied tile!";
      }
      this->_tile->setDie(this);
    }
  }

  void Die::setTileNext(Direction d) {
    int x = this->_tile->getX();
    int y = this->_tile->getY();
    switch (d) {
    case NORTH:
      y++;
      break;
    case SOUTH:
      y--;
      break;
    case EAST:
      x++;
      break;
    case WEST:
      x--;
      break;
    }
    this->setTile(this->_scene->getTile(x, y));
  }

  /// get the tile belonging to this die
  /**
     \return tile currently associated to this die
  */
  Tile* Die::getTile() {
    return this->_tile;
  }

  /// render the die
  void Die::_render() {
    this->_animate();

    // setting the color is necessary in order to ensure that the texture is drawn 'as-is'
    // leaving this out might cause the texture to be drawn with 'shaded' colors
    // because all texture-pixel rgb values are multiplied with the corresponding values
    // of the current color before being drawn!
    int b = 0;
    if (this->_playColor == BLACK) {
      b = 1;
    }
    if ( !this->_scene->inObjPickingMode) {
      glEnable(GL_TEXTURE_2D);
    }

    // standard orientation of faces:
    //  +x: 4; -x: 3
    //  +y: 5; -y: 2
    //  +z: 6; -z: 1

    // face 1
    if ( !this->_scene->inObjPickingMode) {
      if (this->IS_KING) {
        glBindTexture(GL_TEXTURE_2D, Die::textures[12 + b]);
      } else {
        glBindTexture(GL_TEXTURE_2D, Die::textures[0 + b]);
      }
    }
    glBegin(GL_QUADS);
    glNormal3f( 0., 0., -1.);
    glTexCoord2f(0.0, 1.0);
    glVertex3f( -0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( +0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( +0.5, +0.5, -0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( -0.5, +0.5, -0.5);
    glEnd();
    // face 2
    if ( !(this->_scene->inObjPickingMode || this->IS_KING)) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[2 + b]);
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glNormal3f( 0., -1., 0.);
    glVertex3f( -0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( +0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( +0.5, -0.5, +0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( -0.5, -0.5, +0.5);
    glEnd();
    // face 3
    if ( !(this->_scene->inObjPickingMode || this->IS_KING)) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[4 + b]);
    }
    glBegin(GL_QUADS);
    glNormal3f( -1., 0., 0.);
    glTexCoord2f(0.0, 1.0);
    glVertex3f( -0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( -0.5, +0.5, -0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( -0.5, +0.5, +0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( -0.5, -0.5, +0.5);
    glEnd();
    // face 4
    if ( !(this->_scene->inObjPickingMode || this->IS_KING)) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[6 + b]);

    }
    glBegin(GL_QUADS);
    glNormal3f( 1., 0., 0.);
    glTexCoord2f(0.0, 1.0);
    glVertex3f( +0.5, -0.5, -0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( +0.5, +0.5, -0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( +0.5, +0.5, +0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( +0.5, -0.5, +0.5);
    glEnd();
    // face 5
    if ( !(this->_scene->inObjPickingMode || this->IS_KING)) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[8 + b]);
    }
    glBegin(GL_QUADS);
    glNormal3f( 0., 1., 0.);
    glTexCoord2f(0.0, 1.0);
    glVertex3f( -0.5, +0.5, -0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( +0.5, +0.5, -0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( +0.5, +0.5, +0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( -0.5, +0.5, +0.5);
    glEnd();
    // face 6
    if ( !(this->_scene->inObjPickingMode || this->IS_KING)) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[10 + b]);
    }
    glBegin(GL_QUADS);
    glNormal3f( 0., 0., 1.);
    glTexCoord2f(0.0, 1.0);
    glVertex3f( -0.5, -0.5, +0.5);
    glTexCoord2f(1.0, 1.0);
    glVertex3f( +0.5, -0.5, +0.5);
    glTexCoord2f(1.0, 0.0);
    glVertex3f( +0.5, +0.5, +0.5);
    glTexCoord2f(0.0, 0.0);
    glVertex3f( -0.5, +0.5, +0.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
  }

  /// perform changes due to animations
  void Die::_animate() {
    if ( !this->_animationQueue.empty()) {
      this->_scene->enableAutoUpdate();
      Animation* a = this->_animationQueue.front();
      if (a->isFinished()) {
        this->_animationQueue.pop();
        delete a;
        this->_animate();
      } else {
        a->progress();
      }
    } else {
      this->_isMoving = false;
      this->_scene->disableAutoUpdate();
    }
  }

  const Vec Die::RollAnimation::_radials[] = {
    Vec(), // dummy for base-1 array (Directions are base-1)
    Vec(0.0f, 0.5f, -0.5f), // NORTH
    Vec(0.0f, -0.5f, -0.5f), // SOUTH
    Vec(0.5f, 0.0f, -0.5f), // EAST
    Vec( -0.5f, 0.0f, -0.5f) // WEST
  };


  Die::Animation::Animation(Die& die)
    : _parent(die) {
  }

  Die::Animation:: ~Animation() {
  }

  Die::KillAnimation::KillAnimation(Die& die) :
    Animation(die),
    _animationIntervall(20),
    _animationSteps(15),
    _stepsDone(0)
  {
    this->_parent._pos.z = 0;
    this->_parent.dissociate();
    this->_timer.start();
  }

  void Die::KillAnimation::progress(){
    if (this->_timer.elapsed() >= this->_animationIntervall) {
      this->_parent._pos.z -= 0.1;
      this->_stepsDone++;
      this->_timer.restart();
    }
  }

  bool Die::KillAnimation::isFinished(){
    if (this->_stepsDone >= this->_animationSteps) {
      this->_parent.setVisibleState(false);
      return true;
    } else {
      return false;
    }
  }

  Die::ResurrectAnimation::ResurrectAnimation(Die& die) :
    Animation(die),
    _animationIntervall(20),
    _animationSteps(15),
    _stepsDone(0)
  {
    this->_parent._pos.z = -1.5;
    this->_parent.setVisibleState(true);
    this->_timer.start();
  }

  void Die::ResurrectAnimation::progress(){
    if (this->_timer.elapsed() >= this->_animationIntervall) {
      if(this->_parent.getTile()->isFree()){
        this->_parent._pos.z += 0.1;
        this->_stepsDone++;
      }
      this->_timer.restart();
    }
  }

  bool Die::ResurrectAnimation::isFinished(){
    if (this->_stepsDone >= this->_animationSteps) {
      this->_parent.reassociate();
      return true;
    } else {
      return false;
    }
  }

  Die::RollAnimation::RollAnimation(Die& die, Direction d) :
    Animation(die),
    _d(d),
    _animationIntervall(20),
    _animationSteps(15),
    _stepsDone(0),
    _rotAngle(0.0f) {
    this->_radial = Die::RollAnimation::_radials[d];
    this->_timer.start();
  }

  void Die::RollAnimation::progress() {
    if (this->_timer.elapsed() >= this->_animationIntervall) {
      Vec p = this->_parent._primaryOrientation;
      Vec s = this->_parent._secondaryOrientation;

      if (this->_stepsDone == 0) {
        this->_anklePoint = this->_parent._pos + this->_radial;
        float a = 90.0f / this->_animationSteps;
        switch (this->_d) {
        case NORTH:
          this->_newPos = this->_parent._pos + NormalVectors::Y;
          this->_rotAxis = NormalVectors::X.rotate(p, s, p.cross(s));
          this->_rotAngle = a;
          break;
        case SOUTH:
          this->_newPos = this->_parent._pos - NormalVectors::Y;
          this->_rotAxis = NormalVectors::X.rotate(p, s, p.cross(s));
          this->_rotAngle = -a;
          break;
        case EAST:
          this->_newPos = this->_parent._pos + NormalVectors::X;
          this->_rotAxis = NormalVectors::Y.rotate(p, s, p.cross(s));
          this->_rotAngle = -a;
          break;
        case WEST:
          this->_newPos = this->_parent._pos - NormalVectors::X;
          this->_rotAxis = NormalVectors::Y.rotate(p, s, p.cross(s));
          this->_rotAngle = a;
          break;
        }
      }

      switch (this->_d) {
      case NORTH:
      case SOUTH:
        s = s.rotate(this->_rotAxis, this->_rotAngle);
        this->_radial = this->_radial.rotate(NormalVectors::X, -this->_rotAngle);
        break;
      case EAST:
      case WEST:
        p = p.rotate(this->_rotAxis, this->_rotAngle);
        this->_radial = this->_radial.rotate(NormalVectors::Y, -this->_rotAngle);
        break;
      }
      this->_parent._pos = this->_anklePoint - this->_radial;
      this->_parent.setOrientation(p, s);
      this->_stepsDone++;
      this->_timer.restart();
    }
  }

  bool Die::RollAnimation::isFinished() {
    if (this->_stepsDone >= this->_animationSteps) {
      // minimize accumulating rounding errors by setting position
      // explicitly after last animation frame
      this->_parent._pos = this->_newPos;
      // update tile after move
      this->_parent.setTileNext(this->_d);
      return true;
    } else {
      return false;
    }
  }

  void Die::addAnimation(Animation* a){
    this->_animationQueue.push(a);
  }

  void Die::rollOverFields(RelativeMove relMove) {
    this->_isMoving = true;
    // generate vertical and horizontal roll animations
    std::queue< Die::RollAnimation* > horizontal;
    std::queue< Die::RollAnimation* > vertical;
    Direction d;
    if (relMove.dx < 0) {
      d = WEST;
    } else {
      d = EAST;
    }
    for (int i = 0; i < abs(relMove.dx); i++) {
      horizontal.push(new Die::RollAnimation( *this, d));
    }
    if (relMove.dy < 0) {
      d = SOUTH;
    } else {
      d = NORTH;
    }
    for (int i = 0; i < abs(relMove.dy); i++) {
      vertical.push(new Die::RollAnimation( *this, d));
    }
    // add single roll animations to animation queue
    if (relMove.firstX) {
      while ( !horizontal.empty()) {
        this->_animationQueue.push(horizontal.front());
        horizontal.pop();
      }
      while ( !vertical.empty()) {
        this->_animationQueue.push(vertical.front());
        vertical.pop();
      }
    } else {
      while ( !vertical.empty()) {
        this->_animationQueue.push(vertical.front());
        vertical.pop();
      }
      while ( !horizontal.empty()) {
        this->_animationQueue.push(horizontal.front());
        horizontal.pop();
      }
    }
  }

  bool Die::isMoving(){
    return this->_isMoving;
  }

  Color Path::NORMAL_COLOR = ColorTable::YELLOW;

  Path::Path(Scene* scene, Vec posFrom, Move move)
    : Model(scene, posFrom),
      _move(move) {
  }

  Move Path::getMove() {
    return this->_move;
  }

  //Path::Path(Scene* scene, Vec posFrom, RelativeMove relMove, bool isMainPath)
  //    : Model(scene, posFrom),
  //      _move(relMove),
  //      _isMainPath(isMainPath) {
  //}

  void Path::_setColor() {
    // set correct color
    //  if (this->_isMainPath) {
    //    Path::MAIN_COLOR.setAsGlColor();
    //  } else {
    Path::NORMAL_COLOR.setAsGlColor();
    //  }
  }

  void Path::_render() {

    //FIXME: still some errors here!

    if ((this->_move.rel.dx == 0) && (this->_move.rel.dy == 0)) {
      // path points to origin; abort
      return;
    }

    // no of edges per 1/4 circle (curve)
    int curveResolution = 2;
    // (half) width of path (in units of 0.5*tile width)
    float w = 0.2f;
    // height over board (should be higher than dice!)
    float h = 0.6f;
    // (half) thickness
    float t = 0.5f * 0.05f;
    // incremental angle to draw curves
    float incrementalAngle = 90.0f / curveResolution;

    // distances in 1. and 2. move direction
    float dx = this->_move.rel.dx;
    float dy = this->_move.rel.dy;
    // use symmetry to draw with same algorithm if
    // move is first in y-direction. i.e. draw inverse move
    // as if it were in x-direction first, then translate appropriately.
    if (this->_move.rel.firstX == false) {
      dx *= -1;
      dy *= -1;
      glTranslatef( -dx, -dy, 0.0f);
    }
    // and signs of first and second move direction
    float sgnX = sgnP(dx);
    float sgnY = sgnP(dy);

    // draw first straight path (to curve or goal)
    if (fabs(dx) > 1.0f) {
      glBegin(GL_QUADS);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(sgnX * 0.5f, w, h+t).setAsGlVertex3f();
      Vec(sgnX * 0.5f, -w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, w, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(sgnX * 0.5f, w, h-t).setAsGlVertex3f();
      Vec(sgnX * 0.5f, -w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, w, h-t).setAsGlVertex3f();
      // side +w
      glNormal3f( 0., 1., 0.);
      Vec(sgnX * 0.5f, w, h+t).setAsGlVertex3f();
      Vec(sgnX * 0.5f, w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, w, h+t).setAsGlVertex3f();
      // side -w
      glNormal3f( 0., -1., 0.);
      Vec(sgnX * 0.5f, -w, h+t).setAsGlVertex3f();
      Vec(sgnX * 0.5f, -w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -w, h+t).setAsGlVertex3f();
      glEnd();
    }

    // draw curve if necessary
    if ((fabs(dy) > 0.0f && fabs(dx) > 0.0f) || (fabs(dx) > 0.0f && fabs(dy) > 0.0f)) {
      glBegin(GL_QUADS);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(dx - sgnX * 0.5f, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, -sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * w, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(dx - sgnX * 0.5f, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, -sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      // sides
      glNormal3f( 0., sgnY, 0.);
      Vec(dx - sgnX * 0.5f, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * w, h+t).setAsGlVertex3f();

      glNormal3f( 0., -sgnY, 0.);
      Vec(dx - sgnX * 0.5f, -sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, -sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, -sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, -sgnY * w, h+t).setAsGlVertex3f();
      glEnd();

      Vec r;
      Vec a(dx - sgnX * w, sgnY * w, h);
      Vec dz(0.0f, 0.0f, t);

      glBegin(GL_TRIANGLE_FAN);
      // top
      glNormal3f( 0., 0., 1.);
      r = Vec(0.0f, sgnY * ( -2.0f * w), 0.0f);
      (a+dz).setAsGlVertex3f();
      for (int i = 0; i <= curveResolution; i++) {
	(a + r + dz).setAsGlVertex3f();
	r = r.rotate(NormalVectors::Z, sgnX * sgnY * incrementalAngle);
      }
      glEnd();
      glBegin(GL_TRIANGLE_FAN);
      // bottom
      glNormal3f( 0., 0., -1.);
      r = Vec(0.0f, sgnY * ( -2.0f * w), 0.0f);
      (a - dz).setAsGlVertex3f();
      for (int i = 0; i <= curveResolution; i++) {
	(a + r - dz).setAsGlVertex3f();
	r = r.rotate(NormalVectors::Z, sgnX * sgnY * incrementalAngle);
      }
      glEnd();
      glBegin(GL_QUADS);
      // sides
      r = Vec(0.0f, sgnY * ( -2.0f * w), 0.0f);
      Vec r_old = r;
      r = r.rotate(NormalVectors::Z, sgnX * sgnY * incrementalAngle);
      //a.setAsGlVertex3f();
      for (int i = 1; i <= curveResolution; i++) {
	//(a + r).setAsGlVertex3f();
	Vec m((r+r_old).normalized());
	glNormal3f( m.x, m.y, m.z);
	(a+r+dz).setAsGlVertex3f();
	(a+r-dz).setAsGlVertex3f();
	(a+r_old-dz).setAsGlVertex3f();
	(a+r_old+dz).setAsGlVertex3f();
	r_old = r;
	r = r.rotate(NormalVectors::Z, sgnX * sgnY * incrementalAngle);
      }
      glEnd();

      glBegin(GL_QUADS);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(dx - sgnX * w, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(dx - sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      // sides
      glNormal3f( -sgnX, 0., 0.);
      Vec(dx - sgnX * w, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      glNormal3f( sgnX, 0., 0.);
      Vec(dx + sgnX * w, sgnY * w, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * w, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      // back
      glNormal3f( 0., sgnY, 0.);
      Vec(dx - sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5, h+t).setAsGlVertex3f();
      glEnd();
    }
    // draw second straight line (if necessary)
    if (fabs(dy) > 1.0f) {
      glBegin(GL_QUADS);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(dx - sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(dx - sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      // side +w
      glNormal3f( sgnX, 0., 0.);
      Vec(dx + sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * w, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      // side -w
      glNormal3f( -sgnX, 0., 0.);
      Vec(dx - sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      // back
      glNormal3f( 0., sgnY, 0.);
      Vec(dx + sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * w, sgnY * 0.5f, h+t).setAsGlVertex3f();
      glEnd();
    }

    // draw arrow head (with reverted symmetry operation)
    if (this->_move.rel.firstX == false) {
      dx *= -1;
      dy *= -1;
      sgnX *= -1;
      sgnY *= -1;
      glTranslatef( -dx, -dy, 0.0f);
    }
    if ((dx == 0) || (dy != 0 && this->_move.rel.firstX)) {
      // vertical
      glBegin(GL_TRIANGLES);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h-t).setAsGlVertex3f();
      glEnd();
      glBegin(GL_QUADS);
      // back
      glNormal3f( 0., -sgnY, 0.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      // sides
      glNormal3f( -8.25335614909678328e-01*sgnX, 5.64642473395035371e-01*sgnY, 0.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h-t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h+t).setAsGlVertex3f();
      glNormal3f( +8.25335614909678328e-01*sgnX, 5.64642473395035371e-01*sgnY, 0.);
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx + sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h-t).setAsGlVertex3f();
      Vec(dx, dy - sgnY * 0.3f, h+t).setAsGlVertex3f();
      glEnd();
    } else {
      // horizontal
      glBegin(GL_TRIANGLES);
      // top
      glNormal3f( 0., 0., 1.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h+t).setAsGlVertex3f();
      // bottom
      glNormal3f( 0., 0., -1.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h-t).setAsGlVertex3f();
      glEnd();
      glBegin(GL_QUADS);
      // back
      glNormal3f( -sgnX, 0., 0.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h+t).setAsGlVertex3f();
      // sides
      glNormal3f( 5.64642473395035371e-01*sgnX, -8.25335614909678328e-01*sgnY, 0.);
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h+t).setAsGlVertex3f();
      glNormal3f( 5.64642473395035371e-01*sgnX, +8.25335614909678328e-01*sgnY, 0.);
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h+t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h-t).setAsGlVertex3f();
      Vec(dx - sgnX * 0.3f, dy, h+t).setAsGlVertex3f();
      glEnd();
    }
  }

  //void Path::setAsMainPath() {
  //  this->_isMainPath = true;
  //}

  //void Path::setAsNormalPath() {
  //  this->_isMainPath = false;
  //}

  /// board constructor
  Board::Board(Scene* scene, size_t nX, size_t nY)
    : Model(scene),
      _nX(nX),
      _nY(nY),
      _tiles(_nX, std::vector< Tile* >(_nY)) {
    // define tile colors
    Color dark = ColorTable::GREY40;
    Color bright = ColorTable::GREY60;
    Color tileColor;
    Vec tilePosition;
    // setup tiles to form a checkered layout
    for (size_t x = 0; x < this->_nX; x++) {
      for (size_t y = 0; y < this->_nY; y++) {
	tileColor = ((x % 2 + y % 2) % 2 == 0) ? dark : bright;
	tilePosition = Vec((float) x - (float) (this->_nX) / 2 + 0.5, (float) y - (float) (this->_nY) / 2 + 0.5, -0.5);
	this->_tiles[x][y] = new Tile(this->_scene, this, tilePosition, x, y, tileColor);
      }
    }
  }

  /// free memory of allocated tiles in destructor
  Board::~Board() {
    std::vector< Tile* >::iterator ys;
    for (size_t x = 0; x < this->_nX; x++) {
      for (ys = this->_tiles[x].begin(); ys != this->_tiles[x].end(); ys++) {
	delete *ys;
      }
      this->_tiles[x].clear();
    }
    this->_tiles.clear();
  }

  /// display board by rendering every tile
  void Board::_render() {
    for (size_t x = 0; x < this->_nX; x++) {
      for (size_t y = 0; y < this->_nY; y++) {
	glLoadName(x + y * this->_nY);
	this->_tiles[x][y]->display();
      }
    }
  }

  /// return tile defined by its coordinates
  Tile* Board::getTile(size_t x, size_t y) {
    if (x < this->_nX && y < this->_nY)
      return this->_tiles[x][y];
    else
      throw "Error: Tile index out of range";
    return NULL;
  }

  /// get the number of tile columns
  size_t Board::getNY() {
    return this->_nY;
  }

  /// get the number of tile rows
  size_t Board::getNX() {
    return this->_nX;
  }

  /// return object by clicked color id
  /**
     \param id id of color clicked
     \return associated object
  */
  Model* Board::clicked(size_t id) {
    for (size_t x = 0; x < this->_nX; x++) {
      for (size_t y = 0; y < this->_nY; y++) {
	Model* obj = this->_tiles[x][y]->clicked(id);
	if (obj) {
	  return obj;
	}
      }
    }
    return NULL;
  }

  /// clear all tile states
  /**
     set all tiles isSelected, isMarked and isHighlighted state to false
  */
  void Board::clearStates() {
    for (size_t x = 0; x < this->_nX; x++) {
      for (size_t y = 0; y < this->_nY; y++) {
	this->_tiles[x][y]->clearStates();
      }
    }
  }

  /// tile constructor
  Tile::Tile(Scene* scene, Board* board, Vec pos, int x, int y, Color color)
    : Model(scene, pos),
      _x(x),
      _y(y),
      basicColor(color),
      _die(NULL),
      _board(board) {
  }

  /// render the tile
  void Tile::_render() {
    Logger log("Tile::_render");
    glBegin(GL_QUADS);
    // upper face
    glNormal3f( 0., 0., 1.);
    glVertex3f( -0.5, -0.5, 0.0);
    glVertex3f(0.5, -0.5, 0.0);
    glVertex3f(0.5, 0.5, 0.0);
    glVertex3f( -0.5, 0.5, 0.0);
    // lower face
    glNormal3f( 0., 0., -1.);
    glVertex3f( -0.5, -0.5, -0.1);
    glVertex3f(0.5, -0.5, -0.1);
    glVertex3f(0.5, 0.5, -0.1);
    glVertex3f( -0.5, 0.5, -0.1);
    // sides
    glNormal3f( 0., -1., 0.);
    glVertex3f( -0.5, -0.5, 0.0);
    glVertex3f(0.5, -0.5, 0.0);
    glVertex3f(0.5, -0.5, -0.1);
    glVertex3f( -0.5, -0.5, -0.1);

    glNormal3f( 0., 1., 0.);
    glVertex3f( -0.5, 0.5, 0.0);
    glVertex3f(0.5, 0.5, 0.0);
    glVertex3f(0.5, 0.5, -0.1);
    glVertex3f( -0.5, 0.5, -0.1);

    glNormal3f( -1., 0., 0.);
    glVertex3f( -0.5, -0.5, 0.0);
    glVertex3f( -0.5, 0.5, 0.0);
    glVertex3f( -0.5, 0.5, -0.1);
    glVertex3f( -0.5, -0.5, -0.1);

    glNormal3f( 1., 0., 0.);
    glVertex3f(0.5, -0.5, 0.0);
    glVertex3f(0.5, 0.5, 0.0);
    glVertex3f(0.5, 0.5, -0.1);
    glVertex3f(0.5, -0.5, -0.1);
    glEnd();
  }

  /// set the tile color before rendering
  void Tile::_setColor() {
    if (this->_isSelected) {
      this->cSelected.setAsGlColor();
      return;
    }
    if (this->_isHighlighted) {
      this->cHighlighted.setAsGlColor();
      return;
    }
    if (this->_isMarked) {
      this->cMarked.setAsGlColor();
      return;
    }
    this->basicColor.setAsGlColor();
  }

  void Tile::setMarkedState(bool marked) {
    if (this->_die)
      this->_die->setMarkedState(marked);
    this->_isMarked = marked;
  }

  void Tile::setSelectedState(bool selected) {
    if (this->_die)
      this->_die->setSelectedState(selected);
    else
      this->_isSelected = selected;
  }

  /// set the die belonging to this tile
  /**
     \param d new die to be associated to this tile
  */
  void Tile::setDie(Die* d) {
    this->_die = d;
  }

  /// get the die belonging to this tile
  /**
     \return die currently associated to this tile
  */
  Die* Tile::getDie() {
    return this->_die;
  }

  int Tile::getX() {
    return this->_x;
  }

  int Tile::getY() {
    return this->_y;
  }

  bool Tile::isFree(){
    return (this->_die == NULL);
  }


  /// clear all object states
  /**
     set all objects isSelected, isMarked and isHighlighted state to false
  */
  void Scene::clearStates() {
    for (size_t id = 0; id < _objList.size(); id++) {
      if (_objList[id]) {
	_objList[id]->clearStates();
      }
    }
  }

  /// wipe all objects from the scene
  void Scene::wipe() {
    for (std::vector< Model* >::iterator obj = this->_objList.begin(); obj != this->_objList.end(); obj++) {
      delete *obj;
    }
    this->_idCounter = 0;
    this->_dice.clear();
    this->_objList.clear();
    this->_board = NULL;
  }

  /// scene constructor
  Scene::Scene(GameWidget* act)
    : Model(this),
      inObjPickingMode(false),
      _act(act),
      _idCounter(0),
      _board(NULL),
      _selected(NULL),
      _cam(Vec(0, -50, 70), Vec(0, 0, 0)),
      _markX(4),
      _markY(4),
      _messages("Scene"),
      _movingDie(KBX::NONE) {
    this->setup();
  }

  size_t Scene::newId(){
    this->_idCounter++;
    return this->_idCounter;
  }

  void Die::setup(DieState& s){
    this->translate(Vec(s.x()-4,s.y()-4,0));
    if(this->getTile()){
      this->getTile()->setDie(NULL);
    } else {
      this->_tile = this->_scene->getTile(s.x(),s.y());
    }
    if(s.gotKilled()){
      this->setVisibleState(false);
    } else {
      this->reassociate();
    }
    //// set orientation according to DieState
    const std::size_t KING = 24;
    const std::size_t KILLED = 25;
    if (s.getCurrentState() != KING && s.getCurrentState() != KILLED) {
      // standard orientation of faces:
      //  +x: 4; -x: 3
      //  +y: 5; -y: 2
      //  +z: 6; -z: 1
      s.moveOneStep(SOUTH);
      std::size_t value_north = s.getValue();
      s.moveOneStep(NORTH); // revert move

      s.moveOneStep(WEST);
      std::size_t value_east = s.getValue();
      s.moveOneStep(EAST); // revert move

      assert(value_north > 0 && value_north < 7);
      assert(value_east > 0 && value_east < 7);

      // setOrientation always rotates first and second
      // vectors (given in model-internal coordinates)
      // onto the x- and y-axes respectively (in global coordinates, i.e. EAST and NORTH (from white's POV)).
      // e.g.: rotate die, so that 'one' points to the right and 'three' north
      //       by 'setOrientation(NormalVectors::Z * (-1), NormalVectors::X * (-1));'
      std::map<std::size_t, Vec> rot_direction;
      rot_direction[1] = NormalVectors::Z * (-1);
      rot_direction[2] = NormalVectors::Y * (-1);
      rot_direction[3] = NormalVectors::X * (-1);
      rot_direction[4] = NormalVectors::X;
      rot_direction[5] = NormalVectors::Y;
      rot_direction[6] = NormalVectors::Z;
      this->setOrientation(rot_direction[value_east], rot_direction[value_north]);
    }
  }

  void Scene::setupFromGame(Game* game) {
    this->wipe();
    
    // initialize the board and add it to the scene
    this->_board = new Board(this, 9, 9);
    this->add(this->_board);
    this->_markX = 4;
    this->_markY = 4;
    this->_selected = NULL;
    for (size_t i = 0; i < game->getNumberOfDice(); i++) {
      DieState& s = game->getDie(i);
      Die* d = new Die(this,i);
      this->_dice.push_back(d);
      d->setup(s);
      size_t objId = this->add(d);
      this->_dieObjIds[i] = objId;
    }
  }
  

  void Scene::setup() {
    this->wipe();
    // setup dice with correct orientation.
    // per definition, the dice are set up in the same order as the dice
    // are defined in the engine.
    // this way, the id of a single die in the _dice vector and in the engine
    // are the same. from this, the following order follows:
    //      white dice, from left to right (sight from behind white) have ids
    //      0, 1, 2, 3, 4 (king), 5, 6, 7, 8
    //      black dice, from left to right (sight from behind white) have ids
    //      9, 10, 11, 12, 13 (king), 14, 15, 16, 17.

    // initialize the board and add it to the scene
    this->_board = new Board(this, 9, 9);
    this->add(this->_board);
    this->_markX = 4;
    this->_markY = 4;
    this->_selected = NULL;

    // die setup.
    // attention: order has to be kept as is to have same die ids as in engine!
    
    // white dice; w1 is in lower left corner, w8 in lower right
    // black dice; b1 is in upper left corner, b8 in upper right
    for (std::size_t i=0; i < 18; ++i) {
      this->_dice.push_back(new Die(this, i));
      this->_dice.back()->setup(this->_act->_game->getDie(i));
      // add die to scene
      this->_dieObjIds[i] = this->add(this->_dice[i]);
    }
  }

  // scene destructor
  Scene::~Scene() {
    this->wipe();
  }

  void Scene::enableAutoUpdate() {
    this->_act->setAutoUpdate(true);
  }

  void Scene::disableAutoUpdate() {
    this->_act->setAutoUpdate(false);
  }

  void Scene::changed() {
    //  this->_act->setBackgroundColor();
    //  this->display();
    //  this->_act->updateGL();
    this->_act->changed();
  }

  int Scene::movingDie() {
    return this->_movingDie;
  }

  void Scene::setMovingDie(int movingDie) {
    this->_movingDie = movingDie;
  }

  /// render the scene
  void Scene::_render() {
    // clear the graphics buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // switch to the drawing perspective
    glMatrixMode(GL_MODELVIEW);
    // reset the drawing perspective
    glLoadIdentity();
    // set the correct lighting
    if(this->inObjPickingMode){
      glDisable(GL_LIGHTING);
    } else {
      // TODO: make this code better and nicer!
      GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
      GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
      GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
      GLfloat mat_shininess[] = { 50. };
      GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };
      glShadeModel (GL_SMOOTH);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
      glLightfv(GL_LIGHT0, GL_POSITION, light_position);
      glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
      glEnable (GL_COLOR_MATERIAL) ;
      glEnable(GL_LIGHTING);
      glEnable(GL_LIGHT0);
      glEnable(GL_DEPTH_TEST);
    }
    // set correct camera position/view
    this->_cam.updateView();
    // call every object's display method to draw
    // the object to the scene
    for (size_t i = 0; i < this->_objList.size(); i++) {
      if (this->_objList[i]) {
	this->_objList[i]->display();
      }
    }
  }

  void Scene::_setRenderingColor() {
  }

  /// add object to the scene
  /**
     \param obj pointer to a Object to be added to the scene
     \returns unique object id
     \throws string exception, if trying to add null-reference to scene
  */
  size_t Scene::add(Model* obj) {
    if (obj) {
      size_t idx = this->_objList.size();
      this->_objList.push_back(obj);
      return idx;
    } else {
      throw "unable to add object to scene (null-reference)";
    }
  }

  void Scene::remove(size_t objId) {
    delete this->_objList[objId];
    this->_objList[objId] = NULL;
  }

  void Scene::killDie(int dieId){
    size_t objId = this->_dieObjIds[dieId];
    Die* d = dynamic_cast< Die* >(this->_objList[objId]);
    d->addAnimation(new Die::KillAnimation(*d));
  }

  void Scene::resurrectDie(int dieId){
    size_t objId = this->_dieObjIds[dieId];
    Die* d = dynamic_cast< Die* >(this->_objList[objId]);
    d->addAnimation(new Die::ResurrectAnimation(*d));
  }

  Die* Scene::getDie(int dieId) {
    return this->_dice[dieId];
  }

  Tile* Scene::getTile(int x, int y) {
    return this->_board->getTile(x, y);
  }

  /// rotate the scene
  /**
     \param angle the angle around which the scene should be rotated
     \param direction the direction of the rotation, either Camera.HORIZONTAL or Camera.VERTICAL
  */
  void Scene::rotate(float angle, size_t direction) {
    this->_cam.rotate(angle, direction);
  }

  /// obtain camera orientation relative to center of scene
  Vec Scene::getOrientation() {
    return this->_cam.getOrientation();
  }

  /// zoom the scene in/out
  /**
     \param factor the zoom factor
  */
  void Scene::zoom(float factor) {
    this->_cam.zoom(factor);
  }

  /// display in picking mode
  /**
     renders the scene in picking-mode. all textures are disabled and
     all object are drawn in a unique color to make them identifyable
     by the color of the pixel clicked.
  */
  void Scene::display_picking() {
    this->inObjPickingMode = true;
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    this->display();
    inObjPickingMode = false;
  }

  /// pick Object at given mouse coordinates
  /**
     \param p mouse coordinates
     \returns Object* to object under mouse cursor
  */
  Model* Scene::pickObject(QPoint p) {
    // get resolution from settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // get color information from frame buffer
    unsigned char pixel[3];
    this->display_picking();
    // Important: gl (0,0) is bottom left but window coords (0,0) are top left -> have to subtract y from height
    glReadPixels(p.x(), viewport[3] - p.y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    size_t id = Color(pixel[0], pixel[1], pixel[2]).id();
    this->_messages.info(stringprintf("clicked object id: %d", (int) id));
    Model* obj = this->clicked(id);
    if (obj) {
      return obj;
    } else {
      this->_messages.warning(stringprintf("sorry, could not find object matching id %d", (int) id));
      return NULL;
    }
  }

  /// return object by clicked color id
  /**
     \param id id of color clicked
     \return associated object
  */
  Model* Scene::clicked(size_t id) {
    for (size_t i = 0; i < this->_objList.size(); i++) {
      if (this->_objList[i]) {
	Model* obj = this->_objList[i]->clicked(id);
	if (obj) {
	  return obj;
	}
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
  void Scene::markNext(Vec delta) {
    if ( !this->_board) {
      this->_messages.warning("Scene::markNext called without board!");
      return;
    }
    int dx, dy;
    if (fabs(delta.y) >= fabs(delta.x)) {
      dy = sgn(delta.y);
      dx = 0;
    } else {
      dy = 0;
      dx = sgn(delta.x);
    }
    this->_board->getTile(this->_markX, this->_markY)->setMarkedState(false);
    if (this->_markX + dx < this->_board->getNX())
      this->_markX += dx;
    if (this->_markY + dy < this->_board->getNY())
      this->_markY += dy;
    this->_board->getTile(this->_markX, this->_markY)->setMarkedState(true);
  }

  Model* Scene::getMarked() {
    this->clearStates();
    if ( !this->_board) {
      throw "Scene::getMarked called without board!";
    }
    Tile* t = this->_board->getTile(this->_markX, this->_markY);
    if (t->getDie()) {
      return t->getDie();
    } else {
      return t;
    }
  }

  void Scene::setSelected(Model* obj) {
    if (obj) {
      obj->setSelectedState(true);
    }
    this->_selected = obj;
  }

  Model* Scene::getSelected() {
    return this->_selected;
  }
} // end namespace KBX
