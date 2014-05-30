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
#include <queue>
#include <map>
#include <QTime> // for animations
// forward declaration for KBX::Scene
namespace KBX {
class GameWidget;
}

#include "engine.hpp"
#include "tools.hpp"

namespace KBX {
/// defines a camera object for opengl
class Camera {
    Vec _target;
    Vec _position;
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

// Model
//  abstract class defining opengl object
class Model {
  public:
    static void loadTexture(QString filename, GLuint* textureIds, size_t nId);
    static Color cSelected;
    static Color cHighlighted;
    static Color cMarked;
    // constructor
    Model(Scene* scene);
    Model(Scene* scene, Vec pos);
    virtual ~Model();
    virtual Model* clicked(size_t id);
    virtual void clearStates();
    void setMarkedState(bool marked);
    void setSelectedState(bool selected);
    void setHighlightedState(bool highlighted);
    // sets angle and axis to define a rotation
    void rotate(Vec axis, float angle);
    // primary maps on x-axis, secondary on y-axis.
    // always assumes a left-handed orthonormal system
    void setOrientation(Vec primary, Vec secondary);
    // sets vector to define a translation
    void translate(Vec direction);
    // render the object and perform translation / rotation
    void display();
    // return position
    Vec getPosition();
    // return the scene
    Scene* getScene();
  protected:
    // primary orientation is per default along positive x-axis
    Vec _primaryOrientation;
    // secondary orientation is per default along positive y-axis
    Vec _secondaryOrientation;
    // origin of model system in global system
    Vec _pos;
    bool _isVisible;
    bool _isHighlighted;
    bool _isMarked;
    bool _isSelected;
    Scene* _scene;
    // define opengl for this object
    virtual void _render() = 0;
    // rotate object to match x-axis onto primaryOrientation and y-axis onto secondaryOrientation
    void _rotate();
    // translate object to _pos
    void _translate();
    virtual void _setRenderingColor();
    virtual void _setColor();
};

// AnimatedModel
//  abstract class defining opengl object with animation support
//class AnimatedModel: public Model {
//  protected:
//    float _angleOrig;
//    float _angleDest;
//    float _rotStep;
//    Vec _posOrig;
//    Vec _posDest;
//    float _transStep;
//    //TODO: overload _rotate, _translate
//    void _animRotate(Vec axis, float angle);
//    void _animTranslate(Vec direction);
//  public:
//    AnimatedModel(Scene* scene);
//    AnimatedModel(Scene* scene, Vec pos);
//    void rotate(Vec axis, float angle) {
//      Model::rotate(axis, angle);
//    }
//    // sets angle and axis to define a rotation
//    void rotate(Vec axis, float angle, float step);
//    // sets vector to define a translation
//    void translate(Vec direction, float step);
//};
// -- AnimObject

/// Die
///  defines a die
///  dieIds are same as in engine; i.e. 0-8 white dice (4 == king), 9-17 black dice (13 == king)
class Die: public Model {
  public:
    const bool IS_KING;
    Die(Scene* scene, Vec pos, size_t dieId);

    void setTile(Tile* t);
    void setTileNext(Direction d);
    Tile* getTile();

    size_t getId();
    PlayColor getPlayColor();

    void rollOverFields(RelativeMove relMove);
    bool isMoving();

  private:
    static GLuint textures[];
    static bool texturesLoaded;
    static void loadTextures();

    void _render();
    Tile* _tile;
    size_t _dieId;
    PlayColor _playColor;

  public:
    class Animation {
      protected:
        Die& _parent;
      public:
	Animation(Die& d);
	virtual ~Animation();
        virtual void progress() = 0;
        virtual bool isFinished() = 0;
    };

    class RollAnimation : public Animation{
        static const Vec _radials[];

        Direction _d;
        QTime _timer;
        int _animationIntervall; // [ms]
        int _animationSteps;
        int _stepsDone;
        Vec _rotAxis;
        float _rotAngle; // [deg]
        Vec _anklePoint;
        Vec _radial;
        Vec _newPos;
      public:
        RollAnimation(Die& die, Direction d);
        void progress() override;
        bool isFinished() override;
    };

    class KillAnimation : public Animation{
        QTime _timer;
        int _animationIntervall; // [ms]
        int _animationSteps;
        int _stepsDone;

      public:
        KillAnimation(Die& die);
        void progress() override;
        bool isFinished() override;
    };

  private:
    std::queue< Animation* > _animationQueue;
    void _animate();
    bool _isMoving;

  public:
    void addAnimation(Animation* a);


};

/// Path
///  draw a path of possible moves for a die
class Path: public Model {
  public:
    Path(Scene* scene, Vec posFrom, Move move);
    Move getMove();
//    Path(Scene* scene, Vec posFrom, Move move, bool isMainPath);
//    void setAsMainPath();
//    void setAsNormalPath();
    static Color NORMAL_COLOR;
  private:
    //    static const Color MAIN_COLOR;
    Move _move;
    //    bool _isMainPath;
    void _render();
    // overwrite general color setting
    void _setColor();
};

/// Board Tile
///  defines game board tile
class Tile: public Model {
  public:
    Tile(Scene* scene, Board* board, Vec pos, int x, int y, Color color);

    Die* getDie();
    void setDie(Die* d);

    int getX();
    int getY();

    void setMarkedState(bool marked);
    void setSelectedState(bool selected);
  private:
    int _x;
    int _y;

    void _render();
    Color basicColor;
    void _setColor();

    Die* _die;
    Board* _board;
};

/// Board
///  defines game board
class Board: public Model {
  public:
    Board(Scene* scene, size_t rows, size_t cols);
    ~Board();

    size_t getNX();
    size_t getNY();

    void clearStates();
    Model* clicked(size_t id);

    Tile* getTile(size_t x, size_t y);
  private:
    size_t _nX;
    size_t _nY;
    std::vector< std::vector< Tile* > > _tiles;
    void _render();
    void _setRenderingColor() {
    }
};

/// Defines the whole scene.
/**
 Scene is an object container for Objects.
 Every object added to the scene will be rendered,
 when the scene is rendered.
 */
class Scene: public Model {
  public:
    Scene(GameWidget* act);
    ~Scene();
    Vec getOrientation();
    size_t add(Model* obj);
    void remove(size_t objId);
    void removeDie(int dieId);
    void killDie(int dieId);

    Die* getDie(int dieId);
    Tile* getTile(int x, int y);

    void wipe();
    void setup();

    void rotate(float angle, size_t direction);
    void zoom(float factor);
    void clearStates();
    Model* clicked(size_t id);
    Model* pickObject(QPoint p);

    void markNext(Vec delta);
    Model* getMarked();
    Model* getSelected();
    void setSelected(Model* obj);
    void display_picking();
    void enableAutoUpdate();
    void disableAutoUpdate();
    void changed();

    int movingDie();
    void setMovingDie(int dieId);

    bool inObjPickingMode;
    size_t uniqueColorId;
  protected:
    GameWidget* _act;
    std::vector< Model* > _objList;
    Board* _board;
    Model* _selected;
    Camera _cam;
    size_t _markX;
    size_t _markY;
    Logger _messages;
    void _render();
    void _setRenderingColor();
    std::vector< Die* > _dice;
    std::map< int, size_t > _dieObjIds;
    int _movingDie;
};

} // end namespace KBX
#endif
