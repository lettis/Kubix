#include "game_widget.hpp"
#include "tools.hpp"
#include "models.hpp"

#include <GL/glu.h>
#include <QtGui>
#include <QFileDialog>

#include <fstream>
#include <sstream>

namespace KBX {

//TODO: add menu items / preferences for
//      autoRefresh and relativeMarking

void GameWidget::newGame() {
  // clear paths and wipe scene for new game
  // attention: order is important because of
  //            references in path list to
  //            path-objects listed inside scene.
  this->_clearPaths();
  this->_scene->wipe();
  // create new game instance
  // TODO: this is just a dummy for now, please replace by something that makes more sense!
  delete this->_game;
  this->_game = new Game(Config(PlayMode(HUMAN_AI), 3, Strategy(1)));
  // setup board and make change known to renderer
  this->_scene->setup();
  this->changed();
}

GameWidget::GameWidget(QWidget *parent)
    : QGLWidget(parent),
      _log("act"),
      bgColor(ColorTable::GREY10),
      _scene(NULL),
      _autoRefresh(false),
      _autoUpdate(false),
      _updateTimer(NULL),
      _relativeMarking(false),
      _nBuffers(2),
      _bfChange(0) {
  setMouseTracking(false);
  // TODO: this is just a dummy for now, please replace by something that makes more sense!
  this->_game = new Game(Config(PlayMode(HUMAN_AI), 3, Strategy(1)));
}

void GameWidget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  //this->bgColor = Color::GREY20;
  glClearColor(bgColor.r, bgColor.g, bgColor.b, 0.0f);
  this->_scene = new Scene(this);
  // setup timer for graphics refresh
  this->_updateTimer = new QTimer(this);
  connect(this->_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  this->_updateTimer->start(30);
  //this->nBuffers = 2;
  this->setAutoRefresh(false);
  this->changed();
}

/// notify the GLWidget that the scene has changed
void GameWidget::changed() {
  if (this->_autoUpdate || this->_autoRefresh) {
    this->_bfChange = -1;
  } else {
    this->_bfChange = this->_nBuffers;
  }
}

/// notify the GLWidget that the scene has been updated
void GameWidget::_updated() {
  if ( !this->_bfChange == 0) {
    this->_bfChange--;
  }
}

/// ask the GLWidget if the scene needs an update/redraw
/*
 \return true if the scene needs to be redrawn, false otherwise
 */
bool GameWidget::_needUpdate() {
  if (this->_bfChange == 0) {
    return false;
  } else {
    return true;
  }
}

void GameWidget::_clearPaths() {
  for (std::list< size_t >::iterator it = this->_paths.begin(); it != this->_paths.end(); it++) {
    this->_scene->remove( *it);
  }
  this->_paths.clear();
}

/// enable/disable automatic updating of the scene
/*
 \param autoUpdate true if automatic updating should be enabled, false otherwise

 similar to setAutoRefresh, this function controls the automatic redrawing of the scene
 however, while setAutoRefresh reflects user-controlled changes, setAutoUpdate is called
 internally to temporarily enable/disable updating while graphical action is taking place
 such as animated motions of objects
 */
void GameWidget::setAutoUpdate(bool newAutoUpdate) {
  this->_autoUpdate = newAutoUpdate;
  this->changed();
}

/// enable/disable automatic scene refreshing
/**
 \param newAutoRefresh true if automatic scene refreshing is desired, false otherwise

 if set to true, this causes the scene to be redrawn on every timeout of updateTimer
 wasting some GPU power for permanently redrawing a non-changing scene

 on the other hand, if set to false (default), the changed(), updated() and needUpdate()
 methods will be used to automatically keep track of whether the scene needs a redraw
 or not, saving GPU power

 only set to true if there are problems with the graphics
 */
void GameWidget::setAutoRefresh(bool newAutoRefresh) {
  this->_autoRefresh = newAutoRefresh;
  this->changed();
}

void GameWidget::setRelativeMarking(bool newRelativeMarking) {
  this->_relativeMarking = newRelativeMarking;
}

void GameWidget::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  GLfloat aspectRatio = (GLfloat) w / (GLfloat) h;
  gluPerspective(10.0,        // the camera distance
      aspectRatio, // the width-to-height ratio
      1.0,         // the near z clipping coordinate
      1024.0);     // the far z clipping coordinate
  this->changed();
}

void GameWidget::paintGL() {
  if (this->_needUpdate()) {
    glClearColor(this->bgColor.r, this->bgColor.g, this->bgColor.b, 0.0f);
    // if you want to debug the color-picking, use the this->scene->display_picking() instead
    this->_scene->display();
    // set update flag to redraw scene
    this->_updated();
  }
}

/// handle a new mouse press event
/**
 \param event incoming event
 */
void GameWidget::mousePressEvent(QMouseEvent *event) {
  Model* obj;
  if (event->button() == Qt::LeftButton) {
    // pick object
    _scene->clearStates();
    this->userSelect(this->_scene->pickObject(event->pos()));
  } else if (event->button() == Qt::RightButton) {
    // save mouse position for scene rotation
    this->_mousePos = event->pos();
  }
  event->ignore();
  this->changed();
}

/// handle a new mouse move event
/**
 \param event incoming event
 */
void GameWidget::mouseMoveEvent(QMouseEvent *event) {
  float rotAngle = 0.2;
  if (event->buttons() & Qt::RightButton) {
    QPoint delta = event->pos() - this->_mousePos;
    this->_mousePos = event->pos();
    this->_scene->rotate( -1 * delta.x() * rotAngle, Camera::HORIZONTAL);
    this->_scene->rotate(delta.y() * rotAngle, Camera::VERTICAL);
    this->changed();
  } else {
    event->ignore();
  }
}

/// handle a new mouse wheel event
/**
 \param event incoming event
 */
void GameWidget::wheelEvent(QWheelEvent *event) {
  this->_scene->zoom(1 - 0.0005 * event->delta());
  this->changed();
}

/// handle a new key event
/**
 \param event incoming event
 */
void GameWidget::keyPressEvent(QKeyEvent* event) {
  float angle = 5.0;
  float zoom = 0.05;
  switch (event->key()) {
    case Qt::Key_Escape:
      // escape unmarks / delects
      this->_scene->clearStates();
      break;
    case Qt::Key_A:
      this->_scene->rotate( -angle, Camera::HORIZONTAL);
      break;
    case Qt::Key_D:
      this->_scene->rotate(angle, Camera::HORIZONTAL);
      break;
    case Qt::Key_W:
      this->_scene->rotate(angle, Camera::VERTICAL);
      break;
    case Qt::Key_S:
      this->_scene->rotate( -angle, Camera::VERTICAL);
      break;
    case Qt::Key_Q:
      this->_scene->zoom(1.0 + zoom);
      break;
    case Qt::Key_E:
      this->_scene->zoom(1.0 - zoom);
      break;

    case Qt::Key_Up:
      if (this->_relativeMarking) {
        this->_scene->markNext(this->_scene->getOrientation().rotate(Vec(0, 0, 1), 180));
      } else {
        this->_scene->markNext(Vec(0, 1, 0));
      }
      break;
    case Qt::Key_Down:
      if (this->_relativeMarking) {
        this->_scene->markNext(this->_scene->getOrientation().rotate(Vec(0, 0, 1), 0));
      } else {
        this->_scene->markNext(Vec(0, -1, 0));
      }
      break;
    case Qt::Key_Left:
      if (this->_relativeMarking) {
        this->_scene->markNext(this->_scene->getOrientation().rotate(Vec(0, 0, 1), -90));
      } else {
        this->_scene->markNext(Vec( -1, 0, 0));
      }
      break;
    case Qt::Key_Right:
      if (this->_relativeMarking) {
        this->_scene->markNext(this->_scene->getOrientation().rotate(Vec(0, 0, 1), 90));
      } else {
        this->_scene->markNext(Vec(1, 0, 0));
      }
      break;
    case Qt::Key_Space:
      this->userSelect(this->_scene->getMarked());
      break;
    default:
      event->ignore();
      break;
  }
  this->changed();
}

/// handle a user selection event
/**
 \param obj object that has been clicked/selected by the user

 userSelect handles is called whenever a user selects a new object
 either by mouse click, or by keyboard marking.

 this function governs the gameplay!

 userSelect checks if there is already a selected object,
 what kind of object this is
 and what is the corresponding action, for example
 move a die, deselect the currently selected object
 of select a new object and disregard the old selection
 */
void GameWidget::userSelect(Model* obj) {
//  Die* selectedDie = dynamic_cast< Die* >(this->_scene->getSelected());
  Die* selectedDie = dynamic_cast< Die* >(obj);
  if (selectedDie) {
    this->_clearPaths();
    std::list< Move > moves = this->_game->possibleMoves(selectedDie->getId());
    for (std::list< Move >::iterator mv = moves.begin(); mv != moves.end(); mv++) {
      this->_paths.push_back(this->_scene->add(new Path(this->_scene, selectedDie->getPosition(), mv->rel)));
    }
  } else {
    this->_clearPaths();
  }
}

void GameWidget::save() {
  QString ofname = QFileDialog::getSaveFileName(this, "Save game", "", "Kubix Savegames (*.kbx)").simplified();
  if (ofname != "") {
    if ( !ofname.endsWith(".kbx")) {
      ofname += ".kbx";
    }
    //QString ofname = showSaveFileDialog(this, "Save Game", QString(), "Kubix Savegames (*.kbx)");
    std::ofstream outfile(ofname.toStdString().c_str());
    if ( !outfile.is_open()) {
      this->_log.warning("Error: cannot open file to save. Check filesystem permissions!");
      return;
    }
    if (this->_game->write(outfile)) {
      this->_log.info("Saved game successfully to file '" + ofname.toStdString() + "'.");
    } else {
      this->_log.warning("Error saving game!");
    }
    outfile.close();
  }
}

void GameWidget::load() {
  QString ifname = QFileDialog::getOpenFileName(this, "Load Game", "", "Kubix Savegames (*.kbx)").simplified();
  if (ifname != "") {
    std::ifstream infile(ifname.toStdString().c_str());
    if ( !infile.is_open()) {
      this->_log.warning("Error: cannot open file for reading...");
      return;
    }
    if (this->_game->read(infile)) {
      this->_log.info("Loaded game successfully from file '" + ifname.toStdString() + "'.");
    } else {
      this->_log.warning("Error loading game!");
    }
    infile.close();
  }
}

} // end namespace KBX
