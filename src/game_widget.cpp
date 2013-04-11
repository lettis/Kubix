#include "game_widget.hpp"
#include "tools.hpp"
#include "models.hpp"

#include <GL/glu.h>
#include <QtGui>
#include <fstream>
#include <sstream>

/// show a 'Save File'-Dialog
/**
 Qt already offers a function for displaying 'save-file'-dialogs.
 While this works flawlessly under Win and MacOS, automatically appending
 the file extension does not work under *nix - hence this workaround/wrapper.

 courtesy to Dave Mateer for this piece of code
 http://stackoverflow.com/questions/9822177/is-there-a-way-to-automatically-add-extensions-to-a-file-using-qfiledialog-on-li
 */
QString showSaveFileDialog(QWidget *parent, const QString &title, const QString &directory,
    const QString &filter) {
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  return QFileDialog::getSaveFileName(parent,
      title,
      directory,
      filter);
#else
  QFileDialog dialog(parent, title, directory, filter);
  if (parent) {
    dialog.setWindowModality(Qt::WindowModal);
  }
  QRegExp filter_regex(QLatin1String("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
  QStringList filters = filter.split(QLatin1String(";;"));
  if (!filters.isEmpty()) {
    dialog.setNameFilter(filters.first());
    if (filter_regex.indexIn(filters.first()) != -1) {
      dialog.setDefaultSuffix(filter_regex.cap(1));
    }
  }
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() == QDialog::Accepted) {
    QString file_name = dialog.selectedFiles().first();
    QFileInfo info(file_name);
    if (info.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty()) {
      if (filter_regex.indexIn(dialog.selectedNameFilter()) != -1) {
        QString extension = filter_regex.cap(1);
        file_name += QLatin1String(".") + extension;
      }
    }
    return file_name;
  } else {
    return QString();
  }
#endif  // Q_WS_MAC || Q_WS_WIN
}

namespace KBX {

void GameWidget::initializeGUI() {
  QPushButton *btn_newGame = new QPushButton(QApplication::translate("childwidget", "New Game"),
      this);
  btn_newGame->setFocusPolicy(Qt::NoFocus);
  connect(btn_newGame, SIGNAL( released() ), this, SLOT( newGame() ));
  btn_newGame->setToolTip("Start a new game\nThe current game state will be lost.");

  QPushButton *btn_save = new QPushButton(QApplication::translate("childwidget", "Save"), this);
  btn_save->setFocusPolicy(Qt::NoFocus);
  btn_save->move(200, 0);
  connect(btn_save, SIGNAL( released() ), this, SLOT( save() ));
  btn_save->setToolTip("Save the current game to resume playing later.");

  QPushButton *btn_load = new QPushButton(QApplication::translate("childwidget", "Load"), this);
  btn_load->setFocusPolicy(Qt::NoFocus);
  btn_load->move(300, 0);
  connect(btn_load, SIGNAL( released() ), this, SLOT( load() ));
  btn_load->setToolTip("Load the current game to resume playing later.");

  QPushButton *btn_quit = new QPushButton(QApplication::translate("childwidget", "Quit"), this);
  btn_quit->setFocusPolicy(Qt::NoFocus);
  connect(btn_quit, SIGNAL( released() ), this, SLOT( close() ));
  btn_quit->setToolTip("Quit Kubix");
  btn_quit->move(700, 0);

  QCheckBox* chbx_autoRefresh = new QCheckBox(
      QApplication::translate("childwidget", "Auto-Refresh"), this);
  connect(chbx_autoRefresh, SIGNAL( toggled(bool) ), this, SLOT(setAutoRefresh(bool)));
  chbx_autoRefresh->move(400, 0);
  chbx_autoRefresh->setToolTip("Auto-Refresh will cause the scene to be redrawn regularly,"
      " even if nothing changed meanwhile.\nWithout Auto-Refresh,"
      " the scene will only be redrawn if changes happened since the"
      " last redraw.\nOnly enable if you experience flickering"
      " or other graphics issues.");
  chbx_autoRefresh->setFocusPolicy(Qt::NoFocus);
  this->setAutoRefresh(false);

  QCheckBox* chbx_relativeMarking = new QCheckBox(
      QApplication::translate("childwidget", "relative Marking"), this);
  connect(chbx_relativeMarking, SIGNAL( toggled(bool) ), this, SLOT(setRelativeMarking(bool)));
  chbx_relativeMarking->move(550, 0);
  chbx_relativeMarking->setToolTip("Relative Marking will cause the keyboard controls for marking"
      " objects to behave relative to the current camera position.");
  chbx_relativeMarking->setFocusPolicy(Qt::NoFocus);
  this->setRelativeMarking(false);
}

void GameWidget::newGame() {
  this->scene->wipe();
  delete this->game;
  // TODO: this is just a dummy for now, please replace by something that makes more sense!
  this->game = new Game(Config(PlayMode(HUMAN_AI), 3, Strategy(1)));
  this->scene->setup();
}

GameWidget::GameWidget(QWidget *parent)
    : QGLWidget(parent),
      log("act"),
      bgColor(ColorTable::GREY10),
      scene(NULL),
      autoRefresh(false),
      autoUpdate(false),
      updateTimer(NULL),
      relativeMarking(false),
      nBuffers(2),
      bfChange(0) {
  setMouseTracking(false);
  // TODO: this is just a dummy for now, please replace by something that makes more sense!
  this->game = new Game(Config(PlayMode(HUMAN_AI), 3, Strategy(1)));
}

void GameWidget::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  //this->bgColor = Color::GREY20;
  glClearColor(bgColor.r, bgColor.g, bgColor.b, 0.0f);
  this->scene = new Scene(this);
  // setup timer for graphics refresh
  this->updateTimer = new QTimer(this);
  connect(this->updateTimer, SIGNAL(timeout()), this, SLOT(update()));
  this->updateTimer->start(30);
  //this->nBuffers = 2;
  this->setAutoRefresh(false);
  this->changed();
}

/// notify the GLWidget that the scene has changed
void GameWidget::changed() {
  if (this->autoUpdate || this->autoRefresh) {
    this->bfChange = -1;
  } else {
    this->bfChange = this->nBuffers;
  }
}

/// notify the GLWidget that the scene has been updated
void GameWidget::updated() {
  if (!this->bfChange == 0) {
    this->bfChange--;
  }
}

/// ask the GLWidget if the scene needs an update/redraw
/*
 \return true if the scene needs to be redrawn, false otherwise
 */
bool GameWidget::needUpdate() {
  if (this->bfChange == 0) {
    return false;
  } else {
    return true;
  }
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
  this->autoUpdate = newAutoUpdate;
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
  this->autoRefresh = newAutoRefresh;
  this->changed();
}

void GameWidget::setRelativeMarking(bool newRelativeMarking) {
  this->relativeMarking = newRelativeMarking;
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
}

void GameWidget::paintGL() {
  if (this->needUpdate()) {
    glClearColor(this->bgColor.r, this->bgColor.g, this->bgColor.b, 0.0f);
    this->scene->display();
    // if you want to debug the color-picking, use the this->scene->display_picking() instead
    this->updated();
  }
}

/// handle a new mouse press event
/**
 \param event incoming event
 */
void GameWidget::mousePressEvent(QMouseEvent *event) {
  Object* obj;
  if (event->button() == Qt::LeftButton) {
    // pick object
    scene->clearStates();
    this->userSelect(this->scene->pickObject(event->pos()));
  } else if (event->button() == Qt::RightButton) {
    // save mouse position for scene rotation
    this->mousePos = event->pos();
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
    QPoint delta = event->pos() - this->mousePos;
    this->mousePos = event->pos();
    this->scene->rotate(-1 * delta.x() * rotAngle, Camera::HORIZONTAL);
    this->scene->rotate(delta.y() * rotAngle, Camera::VERTICAL);
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
  this->scene->zoom(1 - 0.0005 * event->delta());
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
      this->scene->clearStates();
      break;
    case Qt::Key_A:
      this->scene->rotate(-angle, Camera::HORIZONTAL);
      break;
    case Qt::Key_D:
      this->scene->rotate(angle, Camera::HORIZONTAL);
      break;
    case Qt::Key_W:
      this->scene->rotate(angle, Camera::VERTICAL);
      break;
    case Qt::Key_S:
      this->scene->rotate(-angle, Camera::VERTICAL);
      break;
    case Qt::Key_Q:
      this->scene->zoom(1.0 + zoom);
      break;
    case Qt::Key_E:
      this->scene->zoom(1.0 - zoom);
      break;

    case Qt::Key_Up:
      if (this->relativeMarking) {
        this->scene->markNext(this->scene->getOrientation().rotate(Vec(0, 0, 1), 180));
      } else {
        this->scene->markNext(Vec(0, 1, 0));
      }
      break;
    case Qt::Key_Down:
      if (this->relativeMarking) {
        this->scene->markNext(this->scene->getOrientation().rotate(Vec(0, 0, 1), 0));
      } else {
        this->scene->markNext(Vec(0, -1, 0));
      }
      break;
    case Qt::Key_Left:
      if (this->relativeMarking) {
        this->scene->markNext(this->scene->getOrientation().rotate(Vec(0, 0, 1), -90));
      } else {
        this->scene->markNext(Vec(-1, 0, 0));
      }
      break;
    case Qt::Key_Right:
      if (this->relativeMarking) {
        this->scene->markNext(this->scene->getOrientation().rotate(Vec(0, 0, 1), 90));
      } else {
        this->scene->markNext(Vec(1, 0, 0));
      }
      break;
    case Qt::Key_Space:
      this->userSelect(this->scene->getMarked());
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
 move a die, unselect the currently selected object
 of select a new object and disregard the old selection
 */
void GameWidget::userSelect(Object* obj) {
  if (!obj) {
    this->log.info("Deselecting");
  }
  Die* selectedDie = dynamic_cast< Die* >(this->scene->getSelected());
  Tile* clickedTile = dynamic_cast< Tile* >(obj);
  if (!clickedTile) {
    Die* d = dynamic_cast< Die* >(obj);
    if (d) {
      clickedTile = d->getTile();
    }
  }

  // FIXME dummy paths for testing purposes
  if (clickedTile){
    Vec tilePos = clickedTile->getPosition();
    //TODO: how to delete path afterwards?
    bool x=true;
    RelativeMove relM(2,3,x);
    this->scene->add(new Path(this->scene, tilePos, relM));
    relM = RelativeMove(-2,3,x);
    this->scene->add(new Path(this->scene, tilePos, relM));
    relM = RelativeMove(2,-3,x);
    this->scene->add(new Path(this->scene, tilePos, relM));
    relM = RelativeMove(-2,-3,x);
    this->scene->add(new Path(this->scene, tilePos, relM));
  }


//  if (selectedDie && clickedTile) {
//    //TODO: check if clicked move is valid
//
//    this->log.info("MOVE!!");
//    this->scene->setSelected(NULL);
//  } else {
//    if (obj) {
//      this->log.info("Selecting new object");
//      this->scene->setSelected(obj);
//    }
//  }
}

void GameWidget::save() {
  QString ofname = showSaveFileDialog(this, "Save Game", QString(), "Kubix Savegames (*.kbx)");
  std::ofstream outfile(ofname.toStdString().c_str());
  if (!outfile.is_open()) {
    this->log.warning("Error: cannot open file to save. Check filesystem permissions!");
    return;
  }
  if (this->game->write(outfile)) {
    this->log.info("Saved game successfully to file '" + ofname.toStdString() + "'.");
  } else {
    this->log.warning("Error saving game!");
  }
  outfile.close();
}

void GameWidget::load() {
  QString ifname = QFileDialog::getOpenFileName(this, "Load Game", QString(),
      "Kubix Savegames (*.kbx)");
  std::ifstream infile(ifname.toStdString().c_str());
  if (!infile.is_open()) {
    this->log.warning("Error: cannot open file for reading...");
    return;
  }
  if (this->game->read(infile)) {
    this->log.info("Loaded game successfully from file '" + ifname.toStdString() + "'.");
  } else {
    this->log.warning("Error loading game!");
  }
  infile.close();
}

} // end namespace KBX
