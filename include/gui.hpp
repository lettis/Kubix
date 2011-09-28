#ifndef GUI__HPP
#define GUI__HPP

#include "SDL_opengl.h"

#include <vector>

void loadTexture(char* filename);

void initOpenGL();
void initSDL();

// KBX_Vec
//  defines a simple vector in 3d-cartesian coordinates
class KBX_Vec{
public:
	float x;
	float y;
	float z;
	KBX_Vec();
	float norm();
	void normalize();
};

// KBX_Object
//  abstract class defining opengl object
class KBX_Object{
protected:
	float   _angle;
	KBX_Vec _rotAxis;
	KBX_Vec _pos;
	bool    _isVisible;
	// define opengl for this object
	virtual void _render() = 0;
	// rotate object around _rotAxis by _angle
	void _rotate();
	// translate object to _pos
	void _translate();
public:
	// constructor
	KBX_Object();
	// virtual destructor
	virtual ~KBX_Object() {}
	// sets angle and axis to define a rotation
	void rotate(KBX_Vec axis, float angle);
	// sets vector to define a translation
	void translate(KBX_Vec direction);
	// render the object and perform translation / rotation
	void display();
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
    void rotate(KBX_Vec axis, float angle){
        KBX_Object::rotate(axis, angle);
    }
	// sets angle and axis to define a rotation
	void rotate(KBX_Vec axis, float angle, float step);
	// sets vector to define a translation
	void translate(KBX_Vec direction, float step);
};
// -- KBX_AnimObject


// KBX_Die
//  defines a die
class KBX_Die : public KBX_AnimObject{
	void _render();
	GLuint textures[6];
public:
	KBX_Die();
};


// KBX_Board
//  defines game board
class KBX_Board: public KBX_Object{
	void _render();
};

/// Defines the whole scene.
/**
    KBX_Scene is an object container for KBX_Objects.
    Every object added to the scene will be rendered,
    when the scene is rendered.
*/
class KBX_Scene : public KBX_Object{
    std::vector<KBX_Object*> objList;
    void _render();
    void _rotate();
public:
    void add(KBX_Object* obj);
    KBX_Scene();
};
#endif
