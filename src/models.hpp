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

#include <QTime> // for animations

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
  protected:
    // primary orientation is per default along positive x-axis
    Vec _primaryOrientation;
    // secondary orientation is per default along positive y-axis
    Vec _secondaryOrientation;
    // origin of model system in global system
    Vec _pos;

    bool _isVisible;
    Scene* _scene;
    bool _isSelected;
    bool _isHighlighted;
    bool _isMarked;
    // define opengl for this object
    virtual void _render() = 0;
    // rotate object to match x-axis onto primaryOrientation and y-axis onto secondaryOrientation
    void _rotate();
    // translate object to _pos
    void _translate();
    virtual void _setRenderingColor();
    virtual void _setColor();
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
  private:
    static GLuint textures[];
    static bool texturesLoaded;
    static void loadTextures();

    void _render();
    Tile* _tile;
    size_t _dieId;
    PlayColor _playColor;

    class RollAnimation {
        Die& _parent;
        Direction _d;
        QTime _timer;
        int _animationIntervall; // [ms]
        int _animationSteps;
        int _stepsDone;
        Vec _rotAxis;
        float _rotAngle; // [deg]
      public:
        RollAnimation(Die& die, Direction d);
        void progress();
        bool isFinished();
    };

    std::queue<RollAnimation> _animationQueue;
    void _animate();

  public:
    const bool IS_KING;

    void setTile(Tile* t);
    Tile* getTile();

    Die(Scene* scene, Vec pos, size_t dieId);
    size_t getId();

    void rollOneField(Direction d);
};

/// Path
///  draw a path of possible moves for a die
class Path: public Model {
    static const Color MAIN_COLOR;
    static const Color NORMAL_COLOR;
    RelativeMove _relMove;
    bool _isMainPath;
    void _render();
    // overwrite general color setting
    void _setColor();
  public:
    Path(Scene* scene, Vec posFrom, RelativeMove relMove);
    Path(Scene* scene, Vec posFrom, RelativeMove relMove, bool isMainPath);
    void setAsMainPath();
    void setAsNormalPath();
};

/// Board Tile
///  defines game board tile
class Tile: public Model {
    void _render();
    Color basicColor;
    void _setColor();

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
class Board: public Model {
    size_t _nX;
    size_t _nY;
    std::vector< std::vector< Tile* > > _tiles;
    void _render();
    void _setRenderingColor() {
    }
  public:
    size_t getNX();
    size_t getNY();
    Board(Scene* scene, size_t rows, size_t cols);
    ~Board();

    void clearStates();
    Model* clicked(size_t id);

    Tile* getTile(size_t x, size_t y);
};

/// Defines the whole scene.
/**
 Scene is an object container for Objects.
 Every object added to the scene will be rendered,
 when the scene is rendered.
 */
class Scene: public Model {
  protected:
    GameWidget* _act;

    std::vector< Model* > _objList;

    Board* _board;

    size_t _markX;
    size_t _markY;

    Model* _selected;

    Logger _messages;

    Camera _cam;
    void _render();
    void _setRenderingColor();
    std::vector< Die* > _dice;
//    std::map< size_t, size_t > _objId2Die;

  public:
    Scene(GameWidget* act);
    ~Scene();
    Vec getOrientation();
    size_t add(Model* obj);
    void remove(size_t objId);

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

    bool inObjPickingMode;
    size_t uniqueColorId;
};

} // end namespace KBX
#endif
