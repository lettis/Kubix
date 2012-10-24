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
namespace KBX { class GLWidget; }

// engine is needed for PlayColor definition
#include "engine.hpp"
#include "tools.hpp"

namespace KBX {
  /// define states for gui activity (illumination)
  
  /// defines a simple vector in 3d-cartesian coordinates
  class Vec{
  public:
      float x;
      float y;
      float z;
      Vec();
      Vec(float x, float y, float z);
      float norm();
      Vec normalize();
      Vec scale(float a);
      Vec add(Vec v);
      Vec sub(Vec v);
      Vec rotate(Vec rotAxis, float angle);
      Vec cross(Vec v);
  };
  
  /// defines a color
  class Color{
  public:
      // define some colors fixed
      static const Color BLACK;
      static const Color WHITE;
      static const Color GREY10;
      static const Color GREY20;
      static const Color GREY30;
      static const Color GREY40;
      static const Color GREY50;
      static const Color GREY60;
      static const Color GREY70;
      static const Color GREY80;
      static const Color GREY90;
      static const Color RED;
      static const Color GREEN;
      static const Color BLUE;
      float r;
      float g;
      float b;
      Color();
      Color(size_t id);
      Color(unsigned char r, unsigned char g, unsigned char b);
      Color(int r, int g, int b);
      Color(float r, float g, float b);
      size_t id() const;
      void glColor() const;
  };
  
  /// defines a camera object for opengl
  class Camera{
      Vec target;
      Vec position;
  public:
      Vec getOrientation();
      static const size_t HORIZONTAL=1;
      static const size_t VERTICAL  =2;
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
  // Object
  //  abstract class defining opengl object
  class Object{
  protected:
    std::vector<float>   _angle;
    std::vector<Vec> _rotAxis;
    bool    _isVisible;
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
  };
  
  
  // AnimObject
  //  abstract class defining opengl object with animation support
  class AnimObject : public Object {
  protected:
      float _angleOrig;
      float _angleDest;
      float _rotStep;
      Vec _posOrig;
      Vec _posDest;
      float   _transStep;
      //TODO: overload _rotate, _translate
      void _animRotate(Vec axis, float angle);
      void _animTranslate(Vec direction);
  public:
    AnimObject(Scene* scene);
    AnimObject(Scene* scene, Vec pos);
    void rotate(Vec axis, float angle){
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
  class Die : public AnimObject{
      static GLuint textures[];
      static bool texturesLoaded;
      static void loadTextures();

    void _render();
    PlayColor _playColor;
  public:
      const bool IS_KING;
      Die(Scene* scene, Vec pos, PlayColor color);
      Die(Scene* scene, Vec pos, PlayColor color, bool IS_KING);
  };
  
  /// Board Tile
  ///  defines game board tile
  class Tile: public Object{
    void _render();
    Color basicColor;
    void setColor();
  public:
    Tile(Scene* scene);
    Tile(Scene* scene, Vec pos, Color color);
  };
  
  /// Board
  ///  defines game board
  class Board: public Object{
    size_t _nX;
    size_t _nY;
    std::vector< std::vector<Tile*> > _tiles;
    void _render();
    void _setColor(){};
  public:
    Board(Scene* scene, size_t rows, size_t cols);
    ~Board();
    Object* clicked(size_t id);
    void clearStates();
    // TODO: check if getTileId is needed and reimplement if neccessary
    size_t getTileId(size_t x, size_t y);
  };
  
  /// Defines the whole scene.
  /**
      Scene is an object container for Objects.
      Every object added to the scene will be rendered,
      when the scene is rendered.
  */
  class Scene: public Object{
  protected:
    GLWidget* act;
    
    std::vector<Object*> objList;
    
    Game*  _game;
    Board* _board;
    std::map<size_t, size_t> _id2Die;
    std::map<size_t, std::pair<size_t,size_t> > _id2Field;
    
    Camera cam;
    void _render();
    void _setColor(){};
    
    std::vector<Die*> _dice;

  public:
    Scene(GLWidget* act);
    ~Scene();
    Vec getOrientation();
    void add(Object* obj);
    
    void rotate(float angle, size_t direction);
    void zoom(float factor);
    void clearStates();
    Object* clicked(size_t id);
    Object* pickObject(QPoint p);

    void markNext(int dx, int dy);
    //TODO: implement Scene.select()
    void select();

    bool picking;
    size_t idcount;
  };

} // end namespace KBX
#endif