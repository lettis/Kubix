#include "gui_opengl.hpp"
#include "tools.hpp"
#include "models.hpp"
#include <QTimer>

#include <GL/glu.h>

namespace KBX {

  GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      log("act")
  {
    setMouseTracking(false);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);
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
    // ...and tell the object list that our background color
    // does not correspond to any kind of object
    // Object::objectList.nullId = bgColor.id();
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
    glClearColor(  this->bgColor.r, this->bgColor.g, this->bgColor.b, 0.0f);
    this->scene->display();
    // if you want to debug the color-picking, use the this->scene->display_picking() instead
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
    } else {
      event->ignore();
    }
  }

  void GLWidget::wheelEvent(QWheelEvent *event) {
    Logger l("wheel");
    this->scene->zoom( 1 - 0.0005 * event->delta() );
  }
  
  void GLWidget::keyPressEvent(QKeyEvent* event) {
    float angle = 5.0;
    float zoom  = 0.05;
    switch(event->key()) {
      case Qt::Key_Escape:
        close();
        break;

      case Qt::Key_A:
        this->scene->rotate(  angle, Camera::HORIZONTAL );
        break;
      case Qt::Key_D:
        this->scene->rotate( -angle, Camera::HORIZONTAL );
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
        this->scene->markNext( 0,  1 );
        break;
      case Qt::Key_Down:
        this->scene->markNext( 0, -1 );
        break;
      case Qt::Key_Left:
        this->scene->markNext(-1,  0 );
        break;
      case Qt::Key_Right:
        this->scene->markNext( 1,  0 );
        break;
      case Qt::Key_Space:
        //this->scene->select();

      default:
        event->ignore();
        break;
    }
  }
} // end namespace KBX
