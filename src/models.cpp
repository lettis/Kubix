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
// engine is needed for PlayColor definition
#include "engine.hpp"

namespace KBX {
/// constructor initializing to pos=(0,0,0) and target=(0,1,0)
Camera::Camera() {
  this->position = Vec(0, 0, 0);
  this->target = Vec(0, 1, 0);
}

/// constructor initializing to given pos and target=(0,1,0)
Camera::Camera(Vec pos) {
  this->position = pos;
  this->target = Vec(0, 1, 0);
}

/// constructor initializing to given pos and target
Camera::Camera(Vec pos, Vec target) {
  this->position = pos;
  this->target = target;
}

/// update the OpenGL camera perspective
void Camera::updateView() {
  // set OpenGL camera
  gluLookAt(this->position.x, this->position.y, this->position.z, this->target.x, this->target.y,
      this->target.z, 0, 0, 1);
}

/// get orientation of camera
Vec Camera::getOrientation() {
  return this->position.sub(this->target);
}

/// set new target
/**
 \param target the target to be set (i.e. the point where to look at)
 */
void Camera::setTarget(Vec target) {
  this->target = target;
}

/// set new position
/**
 \param position the new position of the camera
 */
void Camera::setPosition(Vec position) {
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
void Camera::rotate(float angle, size_t direction) {
  Vec v = this->position.sub(this->target);
  if (direction == this->HORIZONTAL) {
    // rotate in horizontal plane, i.e. around the y-axis
    v = v.rotate(Vec(0, 0, 1), angle);
    this->position = this->target.add(v);
  } else if (direction == this->VERTICAL) {
    // rotate in vertical plane, i.e. around the axis
    // orthogonal to the y-axis and the vector v.
    if ((v.normalize().z < 0.99 && angle > 0) || (v.normalize().z > -0.99 && angle < 0)) {
      Vec ortho = v.cross(Vec(0, 0, 1));
      v = v.rotate(ortho, angle);
      this->position = this->target.add(v);
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
  Vec diff = this->position.sub(this->target);
  diff = diff.scale(factor);
  if (diff.norm() < 900 && diff.norm() > 15) {
    this->position = this->target.add(diff);
  }
}

/// load Object textures
/*
 TODO: documentation
 */
void Object::loadTexture(QString filename, GLuint* textures, size_t nTexture) {
  QImage tex, img = QImage(filename);
  tex = QGLWidget::convertToGLFormat(img);
  glBindTexture(GL_TEXTURE_2D, textures[nTexture]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE,
      tex.bits());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/// clear all states
/*
 set isSelected, isMarked and isHighlighted to false
 */
void Object::clearStates() {
  this->setSelectedState(false);
  this->setMarkedState(false);
  this->setHighlightedState(false);
}

// standard colors for marked, selected and highlighted states
Color Object::cSelected = Color(0, 0, 255);
Color Object::cMarked = Color(255, 0, 0);
Color Object::cHighlighted = Color(0, 255, 255);

/// default constructor
Object::Object(Scene* scene)
    : _angle(0),
      _isVisible(true),
      _pos(Vec(0, 0, 0)),
      isHighlighted(false),
      isMarked(false),
      isSelected(false),
      scene(scene)
// we need to add the object to the object list
// and retrieve our own unique id from there
//      ,id(Object::objectList.add(this))
{
}

/// constructor setting the object's position
Object::Object(Scene* scene, Vec pos)
    : _angle(0),
      _isVisible(true),
      _pos(pos),
      isHighlighted(false),
      isMarked(false),
      isSelected(false),
      scene(scene) {
}

/// default destructor
Object::~Object() {
}

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
void Object::rotate(Vec axis, float angle) {
  // push angle to list of successive rotations
  this->_angle.push_back(angle);
  // push axis to list of succ. rotations by converting it
  // to the local frame of reference
  // (this is needed, since the OpenGL coordinate system rotates with the objects)
  if (!this->_rotAxis.empty()) {
    size_t last = this->_rotAxis.size() - 1;
    axis = axis.rotate(this->_rotAxis[last].scale(-1), this->_angle[last]);
  }
  this->_rotAxis.push_back(axis);
}
void Object::setSelectedState(bool selected) {
  this->isSelected = selected;
}
void Object::setMarkedState(bool marked) {
  this->isMarked = marked;
}
void Object::setHighlightedState(bool highlighted) {
  this->isHighlighted = highlighted;
}
Object* Object::clicked(size_t id) {
  if (this->scene->uniqueColorId == id) {
    return this;
  } else {
    this->scene->uniqueColorId++;
    return NULL;
  }
}

/// set object translation
void Object::translate(Vec direction) {
  this->_pos = direction;
}
/// actually rotate object (private, only called by 'display')
void Object::_rotate() {
  for (size_t i = 0; i < this->_angle.size(); i++) {
    glRotatef(this->_angle[i], this->_rotAxis[i].x, this->_rotAxis[i].y, this->_rotAxis[i].z);
  }
}
/// actually translate object (private, only called by 'display')
void Object::_translate() {
  glTranslatef(this->_pos.x, this->_pos.y, this->_pos.z);
}
/// display the object
void Object::display() {
  // save transformations done before
  glPushMatrix();
  this->_setColor();
  this->_translate();
  this->_rotate();
  this->_render();
  // reload transformations done before
  glPopMatrix();
}
void Object::_setColor() {
  if (!this->scene)
    throw "Cannot display object not belonging to any scene!";
  if (this->scene->inObjPickingMode) {
    // render object with unique color mode for object picking
    // (click on obj -> click on unique color -> identify object)
    Color(this->scene->uniqueColorId).setAsGlColor();
    this->scene->uniqueColorId++;
  } else {
    // render object with true color (for view)
    this->setColor();
  }
}

/// set the object color before rendering
void Object::setColor() {
  if (this->isSelected) {
    this->cSelected.setAsGlColor();
    return;
  }
  if (this->isHighlighted) {
    this->cHighlighted.setAsGlColor();
    return;
  }
  if (this->isMarked) {
    this->cMarked.setAsGlColor();
    return;
  }
  Color::WHITE.setAsGlColor();
}

/// inherit parent constructor
AnimObject::AnimObject(Scene* scene)
    : _angleOrig(0),
      _rotStep(0),
      _transStep(0),
      _angleDest(0),
      Object(scene) {
}
/// inherit parent constructor
AnimObject::AnimObject(Scene* scene, Vec pos)
    : _angleOrig(0),
      _rotStep(0),
      _transStep(0),
      _angleDest(0),
      Object(scene, pos) {
}

// set texture loading flag to false initially
bool Die::texturesLoaded = false;
GLuint Die::textures[14];

/// inherit parent constructor and set color
Die::Die(Scene* scene, Vec pos, PlayColor color)
    : AnimObject(scene, pos),
      _playColor(color),
      IS_KING(false),
      _tile(NULL) {
  Die::loadTextures();
}

/// inherit parent constructor and set color
Die::Die(Scene* scene, Vec pos, PlayColor color, bool IS_KING)
    : AnimObject(scene, pos),
      _playColor(color),
      IS_KING(IS_KING),
      _tile(NULL) {
  Die::loadTextures();
}

/**
 load textures for models
 */
void Die::loadTextures() {
  if (!Die::texturesLoaded) {
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

/// get the tile belonging to this die
/**
 \return tile currently associated to this die
 */
Tile* Die::getTile() {
  return this->_tile;
}

/// render the die
void Die::_render() {
  // setting the color is neccessary in order to ensure that the texture is drawn 'as-is'
  // leaving this out might cause the texture to be drawn with 'shaded' colors
  // because all texture-pixel rgb values are multiplied with the corresponding values
  // of the current color before being drawn!
  int b = 0;
  if (this->_playColor == BLACK) {
    b = 1;
  }
  if (!this->scene->inObjPickingMode)
    glEnable(GL_TEXTURE_2D);
  // face 1
  if (!this->scene->inObjPickingMode) {
    if (this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[12 + b]);
    } else {
      glBindTexture(GL_TEXTURE_2D, Die::textures[0 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(+0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(+0.5, +0.5, -0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.5, +0.5, -0.5);
  glEnd();
  // face 2
  if (!this->scene->inObjPickingMode) {
    if (!this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[2 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(+0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(+0.5, -0.5, +0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.5, -0.5, +0.5);
  glEnd();
  // face 3
  if (!this->scene->inObjPickingMode) {
    if (!this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[4 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(-0.5, +0.5, -0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(-0.5, +0.5, +0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.5, -0.5, +0.5);
  glEnd();
  // face 4
  if (!this->scene->inObjPickingMode) {
    if (!this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[6 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(+0.5, -0.5, -0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(+0.5, +0.5, -0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(+0.5, +0.5, +0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(+0.5, -0.5, +0.5);
  glEnd();
  // face 5
  if (!this->scene->inObjPickingMode) {
    if (!this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[8 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.5, +0.5, -0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(+0.5, +0.5, -0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(+0.5, +0.5, +0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.5, +0.5, +0.5);
  glEnd();
  // face 6
  if (!this->scene->inObjPickingMode) {
    if (!this->IS_KING) {
      glBindTexture(GL_TEXTURE_2D, Die::textures[10 + b]);
    }
  }
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex3f(-0.5, -0.5, +0.5);
  glTexCoord2f(1.0, 1.0);
  glVertex3f(+0.5, -0.5, +0.5);
  glTexCoord2f(1.0, 0.0);
  glVertex3f(+0.5, +0.5, +0.5);
  glTexCoord2f(0.0, 0.0);
  glVertex3f(-0.5, +0.5, +0.5);
  glEnd();
  glDisable(GL_TEXTURE_2D);
}

Path::Path(Scene* scene, Vec posFrom, RelativeMove relMove)
    : Object(scene, posFrom),
      _relMove(relMove),
      _isMainPath(false){
}

Path::Path(Scene* scene, Vec posFrom, RelativeMove relMove, bool isMainPath)
    : Object(scene, posFrom),
      _relMove(relMove),
      _isMainPath(isMainPath){
}

void Path::_render() {
  //TODO: finish this
  glBegin(GL_QUADS);
   //TODO: draw quad for straight line
  glEnd();
}

void Path::setAsMainPath() {
  this->_isMainPath = true;
}

void Path::setAsNormalPath() {
  this->_isMainPath = false;
}

/// board constructor
Board::Board(Scene* scene, size_t nX, size_t nY)
    : Object(scene),
      _nX(nX),
      _nY(nY),
      _tiles(_nX, std::vector< Tile* >(_nY)) {
  // define tile colors
  Color dark = Color::GREY40;
  Color bright = Color::GREY60;
  Color tileColor;
  Vec tilePosition;
  // setup tiles to form a checkered layout
  for (size_t x = 0; x < this->_nX; x++) {
    for (size_t y = 0; y < this->_nY; y++) {
      tileColor = ((x % 2 + y % 2) % 2 == 0) ? dark : bright;
      tilePosition = Vec((float) x - (float) (this->_nX) / 2, (float) y - (float) (this->_nY) / 2,
          -0.5);
      this->_tiles[x][y] = new Tile(this->scene, this, x, y, tilePosition, tileColor);
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
Object* Board::clicked(size_t id) {
  Object* obj;
  for (size_t x = 0; x < this->_nX; x++) {
    for (size_t y = 0; y < this->_nY; y++) {
      obj = this->_tiles[x][y]->clicked(id);
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
Tile::Tile(Scene* scene, Board* board, size_t boardX, size_t boardY, Vec pos, Color color)
    : Object(scene, pos),
      basicColor(color),
      _die(NULL),
      _board(board),
      boardX(boardX),
      boardY(boardY) {
}

/// render the tile
void Tile::_render() {
  Logger log("Tile::_render");
  glBegin(GL_QUADS);
  // upper face
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);
  // lower face
  glVertex3f(0.0, 0.0, -0.1);
  glVertex3f(1.0, 0.0, -0.1);
  glVertex3f(1.0, 1.0, -0.1);
  glVertex3f(0.0, 1.0, -0.1);
  // sides
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, -0.1);
  glVertex3f(0.0, 0.0, -0.1);

  glVertex3f(0.0, 1.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(1.0, 1.0, -0.1);
  glVertex3f(0.0, 1.0, -0.1);

  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 1.0, -0.1);
  glVertex3f(0.0, 0.0, -0.1);

  glVertex3f(1.0, 0.0, 0.0);
  glVertex3f(1.0, 1.0, 0.0);
  glVertex3f(1.0, 1.0, -0.1);
  glVertex3f(1.0, 0.0, -0.1);
  glEnd();
}

/// set the tile color before rendering
void Tile::setColor() {
  if (this->isSelected) {
    this->cSelected.setAsGlColor();
    return;
  }
  if (this->isHighlighted) {
    this->cHighlighted.setAsGlColor();
    return;
  }
  if (this->isMarked) {
    this->cMarked.setAsGlColor();
    return;
  }
  this->basicColor.setAsGlColor();
}

void Tile::setMarkedState(bool marked) {
  if (this->_die)
    this->_die->setMarkedState(marked);
  this->isMarked = marked;
}

void Tile::setSelectedState(bool selected) {
  if (this->_die)
    this->_die->setSelectedState(selected);
  else
    this->isSelected = selected;
}
;

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

/// clear all object states
/**
 set all objects isSelected, isMarked and isHighlighted state to false
 */
void Scene::clearStates() {
  for (size_t id = 0; id < objList.size(); id++) {
    objList[id]->clearStates();
  }
}

/// wipe all objects from the scene
void Scene::wipe() {
  for (std::vector< Object* >::iterator obj = this->objList.begin(); obj != this->objList.end();
      obj++) {
    delete *obj;
  }
  this->_dice.clear();
  this->objList.clear();
  this->_board = NULL;
}

/// scene constructor
Scene::Scene(GameWidget* act)
    : Object(this),
      markX(4),
      markY(4),
      selected(NULL),
      inObjPickingMode(false),
      uniqueColorId(0),
      act(act),
      _board(NULL),
      cam(Vec(0, -100, 20), Vec(0, 0, 0)),
      messages("Scene") {
  this->setup();
}

void Scene::setup() {
  // define rotation axes
  Vec toFront(-1.0, 0.0, 0.0);
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
  this->markX = 4;
  this->markY = 4;
  this->selected = NULL;

  // white dice; w1 is in lower left corner, w8 in lower right
  this->_dice.push_back(new Die(this, Vec(-4, -4, 0), WHITE));
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(0, 0));
  this->_dice.push_back(new Die(this, Vec(-3, -4, 0), WHITE));
  this->_dice.back()->rotate(toBack, 90);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(1, 0));
  this->_dice.push_back(new Die(this, Vec(-2, -4, 0), WHITE));
  this->_dice.back()->rotate(toBack, 180);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(2, 0));
  this->_dice.push_back(new Die(this, Vec(-1, -4, 0), WHITE));
  this->_dice.back()->rotate(toFront, 90);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(3, 0));
  this->_dice.push_back(new Die(this, Vec(0, -4, 0), WHITE, true));
  this->_dice.back()->setTile(this->_board->getTile(4, 0));
  this->_dice.push_back(new Die(this, Vec(1, -4, 0), WHITE));
  this->_dice.back()->rotate(toFront, 90);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(5, 0));
  this->_dice.push_back(new Die(this, Vec(2, -4, 0), WHITE));
  this->_dice.back()->rotate(toBack, 180);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(6, 0));
  this->_dice.push_back(new Die(this, Vec(3, -4, 0), WHITE));
  this->_dice.back()->rotate(toBack, 90);
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(7, 0));
  this->_dice.push_back(new Die(this, Vec(4, -4, 0), WHITE));
  this->_dice.back()->rotate(counterClockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(8, 0));

  // black dice; b1 is in upper left corner, b8 in upper right
  this->_dice.push_back(new Die(this, Vec(-4, 4, 0), BLACK));
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(0, 8));
  this->_dice.push_back(new Die(this, Vec(-3, 4, 0), BLACK));
  this->_dice.back()->rotate(toBack, 90);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(1, 8));
  this->_dice.push_back(new Die(this, Vec(-2, 4, 0), BLACK));
  this->_dice.back()->rotate(toBack, 180);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(2, 8));
  this->_dice.push_back(new Die(this, Vec(-1, 4, 0), BLACK));
  this->_dice.back()->rotate(toFront, 90);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(3, 8));
  this->_dice.push_back(new Die(this, Vec(0, 4, 0), BLACK, true));
  this->_dice.back()->setTile(this->_board->getTile(4, 8));
  this->_dice.push_back(new Die(this, Vec(1, 4, 0), BLACK));
  this->_dice.back()->rotate(toFront, 90);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(5, 8));
  this->_dice.push_back(new Die(this, Vec(2, 4, 0), BLACK));
  this->_dice.back()->rotate(toBack, 180);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(6, 8));
  this->_dice.push_back(new Die(this, Vec(3, 4, 0), BLACK));
  this->_dice.back()->rotate(toBack, 90);
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(7, 8));
  this->_dice.push_back(new Die(this, Vec(4, 4, 0), BLACK));
  this->_dice.back()->rotate(clockwise, 90);
  this->_dice.back()->setTile(this->_board->getTile(8, 8));

  // add dice to scene
  for (size_t i = 0; i < this->_dice.size(); i++) {
    this->add(this->_dice[i]);
  }
}

// scene destructor
Scene::~Scene() {
  this->wipe();
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
  this->cam.updateView();
  // call every object's display method to draw
  // the object to the scene
  for (size_t i = 0; i < this->objList.size(); i++) {
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
void Scene::add(Object* obj) {
  if (obj) {
    this->objList.push_back(obj);
  } else {
    throw "unable to add object to scene (null-reference)";
  }
}
/// rotate the scene
/**
 \param angle the angle around which the scene should be rotated
 \param direction the direction of the rotation, either Camera.HORIZONTAL or Camera.VERTICAL
 */
void Scene::rotate(float angle, size_t direction) {
  this->cam.rotate(angle, direction);
}

/// obtain camera orientation relative to center of scene
Vec Scene::getOrientation() {
  return this->cam.getOrientation();
}

/// zoom the scene in/out
/**
 \param factor the zoom factor
 */
void Scene::zoom(float factor) {
  this->cam.zoom(factor);
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
Object* Scene::pickObject(QPoint p) {
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
  this->messages.info(stringprintf("clicked object id: %d", (int) id));
  Object* obj = this->clicked(id);
  if (obj) {
    return obj;
  } else {
    this->messages.warning(stringprintf("sorry, could not find object matching id %d", (int) id));
    return NULL;
  }
}

/// return object by clicked color id
/**
 \param id id of color clicked
 \return associated object
 */
Object* Scene::clicked(size_t id) {
  for (size_t i = 0; i < this->objList.size(); i++) {
    Object* obj = this->objList[i]->clicked(id);
    if (obj) {
      return obj;
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
  if (!this->_board) {
    this->messages.warning("Scene::markNext called without board!");
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
  this->_board->getTile(this->markX, this->markY)->setMarkedState(false);
  if (this->markX + dx < this->_board->getNX())
    this->markX += dx;
  if (this->markY + dy < this->_board->getNY())
    this->markY += dy;
  this->_board->getTile(this->markX, this->markY)->setMarkedState(true);
}

Object* Scene::getMarked() {
  this->clearStates();
  if (!this->_board) {
    throw "Scene::getMarked called without board!";
  }
  Tile* t = this->_board->getTile(this->markX, this->markY);
  if (t->getDie())
    return t->getDie();
  else
    return t;
}

void Scene::setSelected(Object* obj) {
  if (obj) {
    obj->setSelectedState(true);
  }
  this->selected = obj;
}

Object* Scene::getSelected() {
  return this->selected;
}
} // end namespace KBX
