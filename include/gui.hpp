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
#ifndef GUI__HPP
#define GUI__HPP

#include <vector>

#include "SDL_opengl.h"

void initOpenGL(int width, int height);
void initSDL(int width, int height, bool fullscreen);
void setSDLWindow(int width, int height, bool resizeable = true);
void setGLWindow(int width, int height);

class KBX_Vec;
class KBX_Color;
class KBX_Camera;
class KBX_Object;
class KBX_AnimObject;
class KBX_Die;
class KBX_Tile;
class KBX_Board;
class KBX_Scene;

#include "tools.hpp"

/// defines a simple vector in 3d-cartesian coordinates
class KBX_Vec{
public:
    float x;
    float y;
    float z;
    KBX_Vec();
    KBX_Vec(float x, float y, float z);
    float norm();
    KBX_Vec normalize();
    KBX_Vec scale(float a);
    KBX_Vec add(KBX_Vec v);
    KBX_Vec sub(KBX_Vec v);
    KBX_Vec rotate(KBX_Vec rotAxis, float angle);
    KBX_Vec cross(KBX_Vec v);
};

/// defines a color
class KBX_Color{
public:
    float r;
    float g;
    float b;
    KBX_Color();
    KBX_Color(size_t id);
    KBX_Color(unsigned char r, unsigned char g, unsigned char b);
    KBX_Color(int r, int g, int b);
    KBX_Color(float r, float g, float b);
    size_t id();
};

/// defines a camera object for opengl
class KBX_Camera{
    KBX_Vec target;
    KBX_Vec position;
public:
    KBX_Vec getOrientation();
    static const size_t HORIZONTAL=1;
    static const size_t VERTICAL  =2;
    KBX_Camera();
    KBX_Camera(KBX_Vec pos);
    KBX_Camera(KBX_Vec pos, KBX_Vec target);
    void updateView();
    void setTarget(KBX_Vec target);
    void setPosition(KBX_Vec pos);
    void rotate(float angle, size_t direction);
    void zoom(float factor);
};

// KBX_Object
//  abstract class defining opengl object
class KBX_Object{
protected:
    std::vector<float>   _angle;
    std::vector<KBX_Vec> _rotAxis;
    bool    _isVisible;
    KBX_Vec _pos;
    // define opengl for this object
    virtual void _render(bool picking = false) = 0;
    // rotate object around _rotAxis by _angle
    void _rotate();
    // translate object to _pos
    void _translate();
public:
    bool highlighted;
    // a unique id of this object
    const size_t id;
    // the object list handles all KBX_Objects ever constructed
    static KBX_ObjectHandler objectList;
    // constructor
    KBX_Object();
    KBX_Object(KBX_Vec pos);
    // virtual destructor
    virtual ~KBX_Object() {
	// we need to remove the object from the objectList
	KBX_Object::objectList.remove(this);
    }
    // sets angle and axis to define a rotation
    void rotate(KBX_Vec axis, float angle);
    // sets vector to define a translation
    void translate(KBX_Vec direction);
    // render the object and perform translation / rotation
    void display(bool picking = false);
};


// KBX_AnimObject
//  abstract class defining opengl object with animation support
class KBX_AnimObject : public KBX_Object {
protected:
    float _angleOrig;
    float _angleDest;
    float _rotStep;
    KBX_Vec _posOrig;
    KBX_Vec _posDest;
    float   _transStep;
    //TODO: overload _rotate, _translate
    void _animRotate(KBX_Vec axis, float angle);
    void _animTranslate(KBX_Vec direction);
public:
    KBX_AnimObject();
    KBX_AnimObject(KBX_Vec pos);
    void rotate(KBX_Vec axis, float angle){
        KBX_Object::rotate(axis, angle);
    }
    // sets angle and axis to define a rotation
    void rotate(KBX_Vec axis, float angle, float step);
    // sets vector to define a translation
    void translate(KBX_Vec direction, float step);
};
// -- KBX_AnimObject


/// KBX_Die
///  defines a die
class KBX_Die : public KBX_AnimObject{
    void _render(bool picking = false);
    size_t _color;
public:
    // texture container
    static TextureHandler textures;
    // faces of white dice
    static const size_t FACE_K_W;
    static const size_t FACE_K_B;
    static const size_t FACE_1_W;
    static const size_t FACE_2_W;
    static const size_t FACE_3_W;
    static const size_t FACE_4_W;
    static const size_t FACE_5_W;
    static const size_t FACE_6_W;
    // faces of black dice
    static const size_t FACE_1_B;
    static const size_t FACE_2_B;
    static const size_t FACE_3_B;
    static const size_t FACE_4_B;
    static const size_t FACE_5_B;
    static const size_t FACE_6_B;
    // black/white tags
    static const size_t WHITE;
    static const size_t BLACK;
    
    const bool IS_KING;
    KBX_Die(KBX_Vec pos, size_t color);
    KBX_Die(KBX_Vec pos, size_t color, bool IS_KING);
};

/// KBX_Board Tile
///  defines game board tile
class KBX_Tile: public KBX_Object{
    void _render(bool picking = false);
    KBX_Color basicColor;
    KBX_Color activeColor;
public:
    KBX_Tile(KBX_Vec pos, KBX_Color color);
};

/// KBX_Board
///  defines game board
class KBX_Board: public KBX_Object{
    size_t nRows;
    size_t nCols;
    KBX_Tile** tiles;
    void _render(bool picking = false);
public:
    KBX_Board(size_t rows, size_t cols);
};

/// Defines the whole scene.
/**
    KBX_Scene is an object container for KBX_Objects.
    Every object added to the scene will be rendered,
    when the scene is rendered.
*/
class KBX_Scene : public KBX_Object{
    std::vector<KBX_Object*> objList;
    KBX_Camera cam;
    void _render(bool picking = false);
public:
    KBX_Vec getOrientation();
    void add(KBX_Object* obj);
    void rotate(float angle, size_t direction);
    void zoom(float factor);
    KBX_Scene();
};
#endif
