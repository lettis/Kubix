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
    : _primaryOrientation(NormalVectors::X),
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
    : _primaryOrientation(NormalVectors::X),
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
void Model::setMarkedState(bool marked) {
  this->_isMarked = marked;
}
void Model::setHighlightedState(bool highlighted) {
  this->_isHighlighted = highlighted;
}
Model* Model::clicked(size_t id) {
  if (this->_scene->uniqueColorId == id) {
    return this;
  } else {
    this->_scene->uniqueColorId++;
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
    Color(this->_scene->uniqueColorId).setAsGlColor();
    this->_scene->uniqueColorId++;
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
      _dieId(dieId) {
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

PlayColor Die::getPlayColor(){
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

/// set the tile belonging to this die
/**
 \param t new tile to be associated to this die
 */
void Die::setTile(Tile* t) {
  if (this->_tile) {
    if (this->_tile->getDie() != this) {
      throw "Error: trying to dissociate die from unassociated tile!";
    }
    this->_tile->setDie(NULL);
  }
  this->_tile = t;
  if (this->_tile) {
    if (this->_tile->getDie() != NULL) {
      throw "Error: trying to associate die to already occupied tile!";
    }
    this->_tile->setDie(this);
  }
}

void Die::setTileNext(Direction d){
  int x = this->_tile->getX();
  int y = this->_tile->getY();
  switch(d){
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
    if (this->_animationQueue.front().isFinished()) {
      this->_animationQueue.pop();
      this->_animate();
    } else {
      this->_animationQueue.front().progress();
    }
  } else {
    this->_scene->disableAutoUpdate();
  }
}

const Vec Die::RollAnimation::_radials[] = {Vec(), // dummy for base-1 array (Directions are base-1)
    Vec(0.0f, 0.5f, -0.5f), // NORTH
    Vec(0.0f, -0.5f, -0.5f), // SOUTH
    Vec(0.5f, 0.0f, -0.5f), // EAST
    Vec( -0.5f, 0.0f, -0.5f) // WEST
    };

Die::RollAnimation::RollAnimation(Die& die, Direction d)
    : _parent(die),
      _d(d),
      _animationIntervall(20),
      _animationSteps(20),
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

void Die::rollOneField(Direction d) {
  this->_animationQueue.push(Die::RollAnimation( *this, d));
}

void Die::rollOverFields(RelativeMove relMove){
  // generate vertical and horizontal roll animations
  std::queue< Die::RollAnimation > horizontal;
  std::queue< Die::RollAnimation > vertical;
  Direction d;
  if(relMove.dx < 0){
    d = WEST;
  } else {
    d = EAST;
  }
  for(int i=0; i<abs(relMove.dx); i++){
    horizontal.push(Die::RollAnimation( *this, d));
  }
  if(relMove.dy < 0){
    d = SOUTH;
  } else {
    d = NORTH;
  }
  for(int i=0; i<abs(relMove.dy); i++){
    vertical.push(Die::RollAnimation( *this, d));
  }
  // add single roll animations to animation queue
  if(relMove.firstX){
    while(!horizontal.empty()){
      this->_animationQueue.push(horizontal.front());
      horizontal.pop();
    }
    while(!vertical.empty()){
          this->_animationQueue.push(vertical.front());
          vertical.pop();
    }
  } else {
    while(!vertical.empty()){
          this->_animationQueue.push(vertical.front());
          vertical.pop();
    }
    while(!horizontal.empty()){
      this->_animationQueue.push(horizontal.front());
      horizontal.pop();
    }
  }
}


//const Color Path::MAIN_COLOR = ColorTable::GREEN;
const Color Path::NORMAL_COLOR = ColorTable::YELLOW;

Path::Path(Scene* scene, Vec posFrom, Move move)
    : Model(scene, posFrom),
      _move(move){
}

Move Path::getMove(){
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
  // spacer width
  float wSpacer = 0.5f - w;
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

  Vec a(sgnX * w, sgnY * w, h);
  Vec r(0.0f, sgnY * ( -2.0f * w), 0.0f);
  // draw first straight path (to curve or goal)
  if (fabs(dx) > 1.0f) {
    glBegin(GL_QUADS);
    Vec(sgnX * 0.5f, w, h).setAsGlVertex3f();
    Vec(sgnX * 0.5f, -w, h).setAsGlVertex3f();
    Vec(dx - sgnX * 0.5f, -w, h).setAsGlVertex3f();
    Vec(dx - sgnX * 0.5f, w, h).setAsGlVertex3f();
    glEnd();
  }
  // draw curve if necessary
  if ((fabs(dy) > 0.0f && fabs(dx) > 0.0f) || (fabs(dx) > 0.0f && fabs(dy) > 0.0f)) {
    a = Vec(dx - sgnX * 0.5f, sgnY * w, h);

    glBegin(GL_QUADS);
    a.setAsGlVertex3f();
    (a + r).setAsGlVertex3f();
    a = a + Vec(sgnX * wSpacer, 0.0f, 0.0f);
    (a + r).setAsGlVertex3f();
    a.setAsGlVertex3f();
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    a.setAsGlVertex3f();
    for (int i = 0; i <= curveResolution; i++) {
      (a + r).setAsGlVertex3f();
      r = r.rotate(NormalVectors::Z, sgnX * sgnY * incrementalAngle);
    }
    glEnd();

    glBegin(GL_QUADS);
    a.setAsGlVertex3f();
    r = Vec(sgnX * 2.0f * w, 0.0f, 0.0f);
    (a + r).setAsGlVertex3f();
    a = a + Vec(0.0f, sgnY * wSpacer, 0.0f);
    (a + r).setAsGlVertex3f();
    a.setAsGlVertex3f();
    glEnd();
  }

  // draw second straight line (if necessary)
  if (fabs(dy) > 1.0f) {
    a = Vec(dx - sgnX * w, sgnY * 0.5f, h);
    glBegin(GL_QUADS);
    a.setAsGlVertex3f();
    (a + Vec(sgnX * 2.0f * w, 0.0f, 0.0f)).setAsGlVertex3f();
    a = a + Vec(0.0f, sgnY * (fabs(dy) - 1.0f), 0.0f);
    (a + Vec(sgnX * 2.0f * w, 0.0f, 0.0f)).setAsGlVertex3f();
    a.setAsGlVertex3f();
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
    a = Vec(dx - sgnX * 0.5f, dy - sgnY * 0.5f, h);
    glBegin(GL_TRIANGLES);
    a.setAsGlVertex3f();
    (a + Vec(sgnX, 0.0f, 0.0f)).setAsGlVertex3f();
    (a + Vec(0.5f * sgnX, 0.2f * sgnY, 0.0f)).setAsGlVertex3f();
    glEnd();
  } else {
    // horizontal
    a = Vec(dx - sgnX * 0.5f, dy + sgnY * 0.5f, h);
    glBegin(GL_TRIANGLES);
    a.setAsGlVertex3f();
    (a + Vec(0.0f, -sgnY, 0.0f)).setAsGlVertex3f();
    (a + Vec(0.2f * sgnX, -0.5f * sgnY, 0.0f)).setAsGlVertex3f();
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
  glVertex3f( -0.5, -0.5, 0.0);
  glVertex3f(0.5, -0.5, 0.0);
  glVertex3f(0.5, 0.5, 0.0);
  glVertex3f( -0.5, 0.5, 0.0);
  // lower face
  glVertex3f( -0.5, -0.5, -0.1);
  glVertex3f(0.5, -0.5, -0.1);
  glVertex3f(0.5, 0.5, -0.1);
  glVertex3f( -0.5, 0.5, -0.1);
  // sides
  glVertex3f( -0.5, -0.5, 0.0);
  glVertex3f(0.5, -0.5, 0.0);
  glVertex3f(0.5, -0.5, -0.1);
  glVertex3f( -0.5, -0.5, -0.1);

  glVertex3f( -0.5, 0.5, 0.0);
  glVertex3f(0.5, 0.5, 0.0);
  glVertex3f(0.5, 0.5, -0.1);
  glVertex3f( -0.5, 0.5, -0.1);

  glVertex3f( -0.5, -0.5, 0.0);
  glVertex3f( -0.5, 0.5, 0.0);
  glVertex3f( -0.5, 0.5, -0.1);
  glVertex3f( -0.5, -0.5, -0.1);

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

int Tile::getX(){
  return this->_x;
}

int Tile::getY(){
  return this->_y;
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
  this->_dice.clear();
  this->_objList.clear();
  this->_board = NULL;
}

/// scene constructor
Scene::Scene(GameWidget* act)
    : Model(this),
      inObjPickingMode(false),
      uniqueColorId(0),
      _act(act),
      _board(NULL),
      _selected(NULL),
      _cam(Vec(0, -50, 70), Vec(0, 0, 0)),
      _markX(4),
      _markY(4),
      _messages("Scene") {
  this->setup();
}

void Scene::setup() {
  // define rotation axes
  Vec toFront( -1.0, 0.0, 0.0);
  Vec toBack(1.0, 0.0, 0.0);
  Vec toLeft(0.0, 1.0, 0.0);
  Vec toRight(0.0, -1.0, 0.0);
  Vec clockwise(0.0, 0.0, -1.0);
  Vec counterClockwise(0.0, 0.0, 1.0);

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
  this->_dice.push_back(new Die(this, Vec( -4, -4, 0), 0));
  this->_dice.back()->setOrientation(NormalVectors::Z, NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(0, 0));
  this->_dice.push_back(new Die(this, Vec( -3, -4, 0), 1));
  this->_dice.back()->setOrientation(NormalVectors::Y, NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(1, 0));
  this->_dice.push_back(new Die(this, Vec( -2, -4, 0), 2));
  this->_dice.back()->setOrientation(NormalVectors::Z * ( -1), NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(2, 0));
  this->_dice.push_back(new Die(this, Vec( -1, -4, 0), 3));
  this->_dice.back()->setOrientation(NormalVectors::Y * ( -1), NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(3, 0));
  this->_dice.push_back(new Die(this, Vec(0, -4, 0), 4)); // King
  this->_dice.back()->setTile(this->_board->getTile(4, 0));
  this->_dice.push_back(new Die(this, Vec(1, -4, 0), 5));
  this->_dice.back()->setOrientation(NormalVectors::Y * ( -1), NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(5, 0));
  this->_dice.push_back(new Die(this, Vec(2, -4, 0), 6));
  this->_dice.back()->setOrientation(NormalVectors::Z * ( -1), NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(6, 0));
  this->_dice.push_back(new Die(this, Vec(3, -4, 0), 7));
  this->_dice.back()->setOrientation(NormalVectors::Y, NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(7, 0));
  this->_dice.push_back(new Die(this, Vec(4, -4, 0), 8));
  this->_dice.back()->setOrientation(NormalVectors::Z, NormalVectors::X);
  this->_dice.back()->setTile(this->_board->getTile(8, 0));

  // black dice; b1 is in upper left corner, b8 in upper right
  this->_dice.push_back(new Die(this, Vec( -4, 4, 0), 9));
  this->_dice.back()->setOrientation(NormalVectors::Z * ( -1), NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(0, 8));
  this->_dice.push_back(new Die(this, Vec( -3, 4, 0), 10));
  this->_dice.back()->setOrientation(NormalVectors::Y * ( -1), NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(1, 8));
  this->_dice.push_back(new Die(this, Vec( -2, 4, 0), 11));
  this->_dice.back()->setOrientation(NormalVectors::Z, NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(2, 8));
  this->_dice.push_back(new Die(this, Vec( -1, 4, 0), 12));
  this->_dice.back()->setTile(this->_board->getTile(3, 8));
  this->_dice.back()->setOrientation(NormalVectors::Y, NormalVectors::X * ( -1));
  this->_dice.push_back(new Die(this, Vec(0, 4, 0), 13));
  this->_dice.back()->setTile(this->_board->getTile(4, 8));
  this->_dice.push_back(new Die(this, Vec(1, 4, 0), 14));
  this->_dice.back()->setOrientation(NormalVectors::Y, NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(5, 8));
  this->_dice.push_back(new Die(this, Vec(2, 4, 0), 15));
  this->_dice.back()->setOrientation(NormalVectors::Z, NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(6, 8));
  this->_dice.push_back(new Die(this, Vec(3, 4, 0), 16));
  this->_dice.back()->setOrientation(NormalVectors::Y * ( -1), NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(7, 8));
  this->_dice.push_back(new Die(this, Vec(4, 4, 0), 17));
  this->_dice.back()->setOrientation(NormalVectors::Z * ( -1), NormalVectors::X * ( -1));
  this->_dice.back()->setTile(this->_board->getTile(8, 8));

  // add dice to scene
  for (size_t i = 0; i < this->_dice.size(); i++) {
    size_t objId = this->add(this->_dice[i]);
    this->_dieObjIds[i] = objId;
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

/// render the scene
void Scene::_render() {
  // clear the graphics buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // switch to the drawing perspective
  glMatrixMode(GL_MODELVIEW);
  // reset the drawing perspective
  glLoadIdentity();
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
    this->_objList.push_back(obj);
    return this->_objList.size() - 1;
  } else {
    throw "unable to add object to scene (null-reference)";
  }
}

void Scene::remove(size_t objId) {
  delete this->_objList[objId];
  this->_objList[objId] = NULL;
}

void Scene::removeDie(int dieId){
  size_t objId = this->_dieObjIds[dieId];
  Die* d = dynamic_cast< Die* >(this->_objList[objId]);
  d->setTile(NULL);
  this->remove(objId);
}

Tile* Scene::getTile(int x, int y){
  return this->_board->getTile(x,y);
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
  this->uniqueColorId = 0;
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
  this->uniqueColorId = 0;
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
  if (t->getDie())
    return t->getDie();
  else
    return t;
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
