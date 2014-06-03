#include "game_widget.hpp"
#include "tools.hpp"
#include "models.hpp"
#include "config.hpp"
#include "main_window.hpp"

#include <GL/glu.h>
#include <QtGui>
#include <QFileDialog>
#include <QPoint>

#include <fstream>
#include <sstream>

namespace KBX {

Move evaluatorFunc(Game game) {
  return game.evaluateNext();
}

//TODO: add menu items / preferences for
//      autoRefresh and relativeMarking

void GameWidget::newGame(GameConfig c) {
  // clear paths and wipe scene for new game
  // attention: order is important because of
  //            references in path list to
  //            path-objects listed inside scene.
  this->_clearDieSelection();
  this->_scene->wipe();
  // create new game instance
  delete this->_game;
  this->_allowUndoRedo = c.getAllowUndoRedo();
  this->_game = new Game(c);
  // setup board and make change known to renderer
  this->_scene->setup();
  this->changed();
}

GameWidget::GameWidget(QWidget *parent)
    : QGLWidget(parent),
      bgColor(ColorTable::GREY10),
      _scene(NULL),
      _selectedDie(NULL),
      _updateTimer(NULL),
      _autoRefresh(false),
      _autoUpdate(false),
      _allowUndoRedo(true),
      _nBuffers(2),
      _bfChange(0),
      _relativeMarking(false),
      _log("act") {
  setMouseTracking(false);
  // initialize game with some stupid defaults in case there is no config
  Config c(this);
  this->_game = new Game(c);
  // load settings from config
  this->reloadSettings();
  // handle evaluation threads
  connect( &(this->_watcher), SIGNAL(started()), this, SLOT(setEngineRunning()));
  connect( &(this->_watcher), SIGNAL(finished()), this, SLOT(setEngineFinished()));
  connect( &(this->_watcher), SIGNAL(finished()), this, SLOT(performEvaluatedMove()));
}

void GameWidget::cancelEvaluation() {
//TODO shit not working with concurrent::run
  this->_eval.cancel();
}

void GameWidget::undoLastMove(){
  if(!this->_allowUndoRedo) return;
  this->cancelEvaluation();
  int victim = this->_game->getLastMovesVictim();
  Move m = this->_game->undoMove();
  if(m){
    std::cout << "Undo!" << std::endl;
    int dieId = m.dieIndex;
    this->_scene->setMovingDie(dieId);
    KBX::Die* die = this->_scene->getDie(dieId);
    die->rollOverFields(m.rel);
    if(victim != CLEAR) this->_scene->resurrectDie(victim);
  } else {
    std::cout << "Nothing to undo!" << std::endl;
  }
}
void GameWidget::redoLastMove(){
  if(!this->_allowUndoRedo) return;
  this->cancelEvaluation();
  Move m = this->_game->redoMove();
  int victim = this->_game->getLastMovesVictim();
  if(m){
    std::cout << "Redo!" << std::endl;
    int dieId = m.dieIndex;
    this->_scene->setMovingDie(dieId);
    KBX::Die* die = this->_scene->getDie(dieId);
    die->rollOverFields(m.rel);
    if(victim != CLEAR) this->_scene->killDie(victim);
  } else {
    std::cout << "Nothing to redo!" << std::endl;
  }
}


void GameWidget::setBackgroundColor() {
  glClearColor(this->bgColor.r, this->bgColor.g, this->bgColor.b, 0.0f);
}

void GameWidget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  //this->bgColor = Color::GREY20;
  glClearColor(bgColor.r, bgColor.g, bgColor.b, 0.0f);
  this->_scene = new Scene(this);
  // setup timer for graphics refresh
  this->_updateTimer = new QTimer(this);
  connect(this->_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  this->_updateTimer->start(20);
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

void GameWidget::_clearDieSelection() {
  for (std::list< size_t >::iterator it = this->_paths.begin(); it != this->_paths.end(); it++) {
    this->_scene->remove( *it);
  }
  this->_paths.clear();
  this->_selectedDie = NULL;
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
    this->setBackgroundColor();
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
  //FIXME: this function is currently only working with mouse interaction
  KBX::Logger log("userSelect");
  if ( !this->_game->finished()) {
    // dynamically select clicked object
    Die* die = dynamic_cast< Die* >(obj);
    Tile* tile = dynamic_cast< Tile* >(obj);
    Path* path = dynamic_cast< Path* >(obj);

    bool pathIsTempObj = false;
    if (die) {
      if (die->getPlayColor() == this->_game->getNext()) {
        this->_clearDieSelection();
        // die of current player: render paths, i.e. move possibilities
        std::list< Move > moves = this->_game->possibleMoves(die->getId());
        log.info(stringprintf("# of possible moves: %d", moves.size()));
        for (std::list< Move >::iterator mv = moves.begin(); mv != moves.end(); mv++) {
          log.info(
              stringprintf("possible move [dx, dy, firstX]: %d, %d, %s", mv->rel.dx, mv->rel.dy,
                  mv->rel.firstX ? "true" : "false"));
          this->_paths.push_back(this->_scene->add(new Path(this->_scene, die->getPosition(), *mv)));
        }
        this->_selectedDie = die;
      } else {
        if (this->_selectedDie) {
          // die of other player: get tile below die and treat with 'tile-code' below
          tile = die->getTile();
        }
      }
    }
    if (tile) {
      if (this->_selectedDie) {
        int oldX = this->_game->getDie(this->_selectedDie->getId()).x();
        int oldY = this->_game->getDie(this->_selectedDie->getId()).y();
        // check if selected die can move to this field. if yes: move (or draw remaining paths)
        std::list< Move > moves = this->_game->possibleMoves(this->_selectedDie->getId());
        std::list< Move > filteredMoves;
        for (std::list< Move >::iterator mv = moves.begin(); mv != moves.end(); mv++) {
          if ((mv->rel.dx + oldX == tile->getX()) && (mv->rel.dy + oldY == tile->getY())) {
            // move lands on selected tile
            filteredMoves.push_back( *mv);
          }
        }
        if (filteredMoves.size() == 1) {
          // only one move possible: perform move directly
          path = new Path(this->_scene, this->_selectedDie->getPosition(), filteredMoves.front());
          pathIsTempObj = true;
        } else if (filteredMoves.size() == 2) {
          // clear old paths, but keep selected die
          Die* sel = this->_selectedDie;
          this->_clearDieSelection();
          this->_selectedDie = sel;
          // two moves possible: draw paths
          this->_paths.push_back(this->_scene->add(new Path(this->_scene, sel->getPosition(), filteredMoves.front())));
          this->_paths.push_back(this->_scene->add(new Path(this->_scene, sel->getPosition(), filteredMoves.back())));
        } else {
          // no move possible: just clear selection
          this->_clearDieSelection();
        }
      }
    }
    if (path) {
      // move die along this path
      Move mv = path->getMove();
      this->_performMove(mv);
      if (pathIsTempObj && (path != NULL)) {
        delete path;
      }
    }
  }
}

void GameWidget::giveUp() {
  this->_performMove(Move());
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
    outfile << *(this->_game);
    this->_log.info("Saved game to file '" + ofname.toStdString() + "'.");
    outfile.close();
  }
}

void GameWidget::load(std::string ifname){
  if (ifname.size() > 0) {
    std::ifstream infile(ifname);
    if ( !infile.is_open()) {
      this->_log.warning("Error: cannot open file for reading...");
      return;
    }
    //TODO use log instead of cout
    std::cout << "loading game from " << ifname << std::endl;
    infile >> (*this->_game);
    this->_scene->setupFromGame(this->_game);
    infile.close();
  }
}

void GameWidget::load() {
  QString ifname = QFileDialog::getOpenFileName(this, "Load Game", "", "Kubix Savegames (*.kbx)").simplified();
  this->load(ifname.toStdString());
}
 

void GameWidget::reloadSettings() {
//TODO needed? if yes: implement!
}

void GameWidget::_performMove(Move m) {
  if(this->_game->finished()) return;
  QMessageBox msgBox;
  if (m == Move()) {
    // empty move: 'next player' loses
    PlayColor next = this->_game->getNext();
    if (next == this->_game->getAiColor()) {
      msgBox.setText("computer gives up. you win.");
    } else {
      msgBox.setText("you give up. loser!");
    }
    msgBox.exec();
    this->_game->setFinished(true);
  } else {
    int dieId = m.dieIndex;
    int oldX = this->_game->getDie(dieId).x();
    int oldY = this->_game->getDie(dieId).y();
    int capturedDie = this->_game->getDieId(oldX + m.rel.dx, oldY + m.rel.dy);
    if (capturedDie != CLEAR) {
      // remove captured die from board
      this->_scene->killDie(capturedDie);
    }
    this->_scene->setMovingDie(dieId);
    this->_scene->getDie(dieId)->rollOverFields(m.rel);
    // update engine
    this->_game->makeMove(m);
    this->_clearDieSelection();

    // has anybody won?
    PlayColor winner = this->_game->getWinner();
    if (winner != NONE_OF_BOTH) {
      if (winner == this->_game->getAiColor()) {
        msgBox.setText("computer wins.");
      } else if (winner == this->_game->getHumanColor()) {
        msgBox.setText("you win.");
      } else if (winner == WHITE) {
        msgBox.setText("white wins.");
      } else {
        msgBox.setText("black wins.");
      }
      msgBox.exec();
      this->_game->setFinished(true);
    }
  }
}

void GameWidget::performEvaluatedMove(){
  Move m = this->_eval.result();
  this->_performMove(m);
}


void GameWidget::update() {
  //TODO: rewrite this function to fully support all play modes, game states, etc
  if ( !this->_game->finished()) {
    // TODO: quit extra thread when window closes
    if (this->_scene->movingDie() == -1) {
      if (this->_engineMoves() && !this->_watcher.isRunning()) {
        // encapsulate move evaluation by engine
        // in separate thread to keep UI reactive
        this->_eval = QtConcurrent::run(evaluatorFunc, *(this->_game));
        this->_watcher.setFuture(this->_eval);
      }
    } else if ( !this->_scene->getDie(this->_scene->movingDie())->isMoving()) {
      // release lock after die has finished moving
      this->_scene->setMovingDie( -1);
    }
  }
  QGLWidget::update();
}

bool GameWidget::_engineMoves() {
  bool engineMoves = false;
  if (this->_game->playMode() == HUMAN_AI) {
    if (this->_game->getNext() == BLACK) {
      engineMoves = true;
    }
  } else if (this->_game->playMode() == AI_HUMAN) {
    if (this->_game->getNext() == WHITE) {
      engineMoves = true;
    }
  }
  return engineMoves;
}

void GameWidget::setEngineRunning() {
  emit this->newStatus("Kubix is computing next move.");
}

void GameWidget::setEngineFinished() {
  emit this->newStatus("Kubix is waiting for your move.");
}

} // end namespace KBX
