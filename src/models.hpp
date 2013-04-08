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
#ifndef MODELS__HPP
#define MODELS__HPP

#include <vector>

// forward declaration for KBX::Scene
namespace KBX {
class GameWidget;
}

// engine is needed for PlayColor definition
#include "engine.hpp"
#include "tools.hpp"

namespace KBX {
/// defines a camera object for opengl
class Camera {
  Vec target;
  Vec position;
public:
  Vec getOrientation();
  static const size_t HORIZONTAL = 1;
  static const size_t VERTICAL = 2;
  Camera();
  Camera(Vec pos);
  Camera(Vec pos, Vec target);
  void updateView();
  void setTarget(Vec target);
  void setPosition(Vec pos);
  void rotate(float angle, size_t direction);
  void zoom(float factor);
};

class Scene;
class Die;
class Tile;
class Board;

// Object
//  abstract class defining opengl object
class Object {
protected:
  std::vector< float > _angle;
  std::vector< Vec > _rotAxis;
  bool _isVisible;
  Scene* scene;
  bool isSelected;
  bool isHighlighted;
  bool isMarked;
  Vec _pos;
  // define opengl for this object
  virtual void _render() = 0;
  // rotate object around _rotAxis by _angle
  void _rotate();
  // translate object to _pos
  void _translate();
  virtual void _setColor();
  virtual void setColor();
public:
  static void loadTexture(QString filename, GLuint* textureIds, size_t nId);
  static Color cSelected;
  static Color cHighlighted;
  static Color cMarked;
  // constructor
  Object(Scene* scene);
  Object(Scene* scene, Vec pos);
  virtual ~Object();
  virtual Object* clicked(size_t id);
  virtual void clearStates();
  void setMarkedState(bool marked);
  void setSelectedState(bool selected);
  void setHighlightedState(bool highlighted);
  // sets angle and axis to define a rotation
  void rotate(Vec axis, float angle);
  // sets vector to define a translation
  void translate(Vec direction);
  // render the object and perform translation / rotation
  void display();
  // return position
  Vec getPosition();
};

// AnimObject
//  abstract class defining opengl object with animation support
class AnimObject: public Object {
protected:
  float _angleOrig;
  float _angleDest;
  float _rotStep;
  Vec _posOrig;
  Vec _posDest;
  float _transStep;
  //TODO: overload _rotate, _translate
  void _animRotate(Vec axis, float angle);
  void _animTranslate(Vec direction);
public:
  AnimObject(Scene* scene);
  AnimObject(Scene* scene, Vec pos);
  void rotate(Vec axis, float angle) {
    Object::rotate(axis, angle);
  }
  // sets angle and axis to define a rotation
  void rotate(Vec axis, float angle, float step);
  // sets vector to define a translation
  void translate(Vec direction, float step);
};
// -- AnimObject

/// Die
///  defines a die
class Die: public AnimObject {
  static GLuint textures[];
  static bool texturesLoaded;
  static void loadTextures();

  Tile* _tile;

  void _render();
  PlayColor _playColor;
public:
  const bool IS_KING;

  void setTile(Tile* t);
  Tile* getTile();

  Die(Scene* scene, Vec pos, PlayColor color);
  Die(Scene* scene, Vec pos, PlayColor color, bool IS_KING);
};

/// Path
///  draw a path of possible moves for a die
class Path: public Object {
  static const Color MAIN_COLOR;
  static const Color NORMAL_COLOR;
  RelativeMove _relMove;
  bool _isMainPath;
  void _render();
public:
  Path(Scene* scene, Vec posFrom, RelativeMove relMove);
  Path(Scene* scene, Vec posFrom, RelativeMove relMove, bool isMainPath);
  void setAsMainPath();
  void setAsNormalPath();
};

/// Board Tile
///  defines game board tile
class Tile: public Object {
  void _render();
  Color basicColor;
  void setColor();

  Board* _board;
  Die* _die;

public:
  Die* getDie();
  void setDie(Die* d);

  void setMarkedState(bool marked);
  void setSelectedState(bool selected);

  Tile(Scene* scene, Board* board, Vec pos, Color color);
};

/// Board
///  defines game board
class Board: public Object {
  size_t _nX;
  size_t _nY;
  std::vector< std::vector< Tile* > > _tiles;
  void _render();
  void _setColor() {
  }
  ;
public:
  size_t getNX();
  size_t getNY();
  Board(Scene* scene, size_t rows, size_t cols);
  ~Board();
  Object* clicked(size_t id);
  void clearStates();
  Die* getDie(size_t x, size_t y);
  Tile* getTile(size_t x, size_t y);

};

/// Defines the whole scene.
/**
 Scene is an object container for Objects.
 Every object added to the scene will be rendered,
 when the scene is rendered.
 */
class Scene: public Object {
protected:
  GameWidget* act;

  std::vector< Object* > objList;

  //Game*  _game;
  Board* _board;

  size_t markX;
  size_t markY;

  Object* selected;

  Logger messages;

  Camera cam;
  void _render();
  void _setColor();
  std::vector< Die* > _dice;

public:
  Scene(GameWidget* act);
  ~Scene();
  Vec getOrientation();
  size_t add(Object* obj);
  void remove(size_t objId);

  void wipe();
  void setup();

  void rotate(float angle, size_t direction);
  void zoom(float factor);
  void clearStates();
  Object* clicked(size_t id);
  //TODO: write specialized versions: Object* pick(...); Die* pick(...); Tile* pick(...)
  Object* pickObject(QPoint p);

  void markNext(Vec delta);
  Object* getMarked();
  Object* getSelected();
  void setSelected(Object* obj);
  void display_picking();

  bool inObjPickingMode;
  size_t uniqueColorId;
};

} // end namespace KBX
#endif
