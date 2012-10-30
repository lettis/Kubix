#include "gui_opengl.hpp"
#include "tools.hpp"
#include "models.hpp"

#include <GL/glu.h>
#include <QtGui>

namespace KBX {

  void GLWidget::initializeGUI(){
    QPushButton *btn_newGame = new QPushButton(QApplication::translate("childwidget", "New Game"), this);
    btn_newGame->setFocusPolicy( Qt::NoFocus );
    connect(btn_newGame, SIGNAL( released() ), this, SLOT( newGame() ) );
    btn_newGame->setToolTip("Start a new game\nThe current game state will be lost.");

    QPushButton *btn_quit = new QPushButton(QApplication::translate("childwidget", "Quit"), this);
    btn_quit->setFocusPolicy( Qt::NoFocus );
    connect(btn_quit, SIGNAL( released() ), this, SLOT( close() ) );
    btn_quit->setToolTip("Quit Kubix");
    btn_quit->move(700, 0);

    QCheckBox* chbx_autoRefresh = new QCheckBox(QApplication::translate("childwidget", "Auto-Refresh"), this);
    connect( chbx_autoRefresh,  SIGNAL( toggled(bool) ), this, SLOT(setAutoRefresh(bool)));
    chbx_autoRefresh->move(400, 0);
    chbx_autoRefresh->setToolTip("Auto-Refresh will cause the scene to be redrawn regularly, even if nothing changed meanwhile.\nWithout Auto-Refresh, the scene will only be redrawn if changes happened since the last redraw.\nOnly enable if you experience flickering or other graphics issues.");
    chbx_autoRefresh->setFocusPolicy( Qt::NoFocus );
    this->setAutoRefresh(false);

    QCheckBox* chbx_relativeMarking = new QCheckBox(QApplication::translate("childwidget", "relative Marking"), this);
    connect( chbx_relativeMarking,  SIGNAL( toggled(bool) ), this, SLOT(setRelativeMarking(bool)));
    chbx_relativeMarking->move(550, 0);
    chbx_relativeMarking->setToolTip("Relative Marking will cause the keyboard controls for marking objects to behave relative to the current camera position.");
    chbx_relativeMarking->setFocusPolicy( Qt::NoFocus );
    this->setRelativeMarking(false);

  }
  
  void GLWidget::newGame(){
    this->scene->wipe();
    this->scene->setup();
  }

  GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      log("act")
  {
    setMouseTracking(false);
  }

  void GLWidget::initializeGL() {
    glEnable(GL_DEPTH_TEST);
    this->bgColor = Color::GREY20;
    glClearColor(  bgColor.r
                 , bgColor.g
                 , bgColor.b
                 , 0.0f
    );
    this->scene = new Scene(this);

    this->updateTimer = new QTimer(this);
    connect(this->updateTimer, SIGNAL(timeout()), this, SLOT(update()));
    this->updateTimer->start(30);
    this->nBuffers = 2;
    this->setAutoRefresh(false);
    this->changed();
  }

  /// notify the GLWidget that the scene has changed
  void GLWidget::changed(){
    if(this->autoUpdate || this->autoRefresh)
      this->bfChange = -1;
    else
      this->bfChange = this->nBuffers;
  }

  /// notify the GLWidget that the scene has been updated
  void GLWidget::updated(){
    if(! this->bfChange==0) this->bfChange--;
  }


  /// ask the GLWidget if the scene needs an update/redraw
  /*
    \return true if the scene needs to be redrawn, false otherwise
   */
  bool GLWidget::needUpdate(){
    if(this->bfChange == 0) 
      return false;
    return true;
  }

  /// enable/disable automatic updating of the scene
  /*
    \param autoUpdate true if automatic updating should be enabled, false otherwise
    
    similar to setAutoRefresh, this function controls the automatic redrawing of the scene
    however, while setAutoRefresh reflects user-controlled changes, setAutoUpdate is called
    internally to temporarily enable/disable updating while graphical action is taking place
    such as animated motions of objects
  */
  void GLWidget::setAutoUpdate(bool newAutoUpdate){
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

     ony set to true if there are probems with the graphics
  */
  void GLWidget::setAutoRefresh(bool newAutoRefresh){
    this->autoRefresh = newAutoRefresh;
    this->changed();
  }


  void GLWidget::setRelativeMarking(bool newRelativeMarking){
    this->relativeMarking = newRelativeMarking;
  }

  
  void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLfloat aspectRatio = (GLfloat)w / (GLfloat)h;
    gluPerspective(10.0,        // the camera distance
                   aspectRatio, // the width-to-height ratio
                   1.0,         // the near z clipping coordinate
                   1024.0);     // the far z clipping coordinate
  }
  
  void GLWidget::paintGL() {
    if(this->needUpdate()){
      glClearColor(  this->bgColor.r, this->bgColor.g, this->bgColor.b, 0.0f);
      this->scene->display();
      // if you want to debug the color-picking, use the this->scene->display_picking() instead
      this->updated();
    }
  }

  void GLWidget::mousePressEvent(QMouseEvent *event) {
    Object* obj;
    if (event->button() == Qt::LeftButton){
      // pick object
      scene->clearStates();
      obj = this->scene->pickObject( event->pos() );
      if(obj){
        obj->setSelectedState(true);
      }
    } else if (event->button() == Qt::RightButton){
      // save mouse position for scene rotation
      this->mousePos = event->pos();
    } 
    event->ignore();
    this->changed();
  }

  void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    float rotAngle = 0.2;
    if (event->buttons() & Qt::RightButton){
      QPoint delta = event->pos() - this->mousePos;
      this->mousePos = event->pos();
      this->scene->rotate(
        -1 * delta.x() * rotAngle,
        Camera::HORIZONTAL
      );
      this->scene->rotate(
        delta.y() * rotAngle,
        Camera::VERTICAL
      );
      this->changed();
    } else {
      event->ignore();
    }
  }

  void GLWidget::wheelEvent(QWheelEvent *event) {
    Logger l("wheel");
    this->scene->zoom( 1 - 0.0005 * event->delta() );
    this->changed();
  }
  
  void GLWidget::keyPressEvent(QKeyEvent* event) {
    float angle = 5.0;
    float zoom  = 0.05;
    switch(event->key()) {
      case Qt::Key_Escape:
        close();
        break;

      case Qt::Key_A:
        this->scene->rotate( -angle, Camera::HORIZONTAL );
        break;
      case Qt::Key_D:
        this->scene->rotate(  angle, Camera::HORIZONTAL );
        break;
      case Qt::Key_W:
        this->scene->rotate(  angle, Camera::VERTICAL );
        break;
      case Qt::Key_S:
        this->scene->rotate( -angle, Camera::VERTICAL );
        break;
      case Qt::Key_Q:
        this->scene->zoom( 1.0 + zoom );
        break;
      case Qt::Key_E:
        this->scene->zoom( 1.0 - zoom );
        break;

      case Qt::Key_Up:
        this->scene->markNext( this->relativeMarking ? this->scene->getOrientation().rotate(Vec(0,0,1),180) : Vec(0,1,0)); 
        break;
      case Qt::Key_Down:
        this->scene->markNext( this->relativeMarking ? this->scene->getOrientation().rotate(Vec(0,0,1),0) : Vec(0,-1,0)); 
        break;
      case Qt::Key_Left:
        this->scene->markNext( this->relativeMarking ? this->scene->getOrientation().rotate(Vec(0,0,1),-90) : Vec(-1,0,0)); 
        break;
      case Qt::Key_Right:
        this->scene->markNext( this->relativeMarking ? this->scene->getOrientation().rotate(Vec(0,0,1),90) : Vec(1,0,0)); 
        break;
      case Qt::Key_Space:
        this->scene->select();
      default:
        event->ignore();
        break;
    }
    this->changed();
  }
} // end namespace KBX
