#include "gui_opengl.hpp"
#include "tools.hpp"
#include <QTimer>
#include <GL/glu.h>

namespace KBX {
  GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent),
      textures( new Textures() )
  {
    // load textures for models
    this->loadTextures();
    this->scene = new Scene(this);
    setMouseTracking(false);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);
  }

  /**
    load textures for models
  */
  void GLWidget::loadTextures(){
    for ( size_t i=1; i<7; i++ ){
      this->textures->dieTexturesWhite[i-1] = bindTexture(
        QPixmap( QString("./res/side%1.png").arg(i) ),
        GL_TEXTURE_2D
      );
      this->textures->dieTexturesBlack[i-1] = bindTexture(
        QPixmap( QString("./res/side%1b.png").arg(i) ),
        GL_TEXTURE_2D
      );
    }

    this->textures->kingTextureBlack[0] = bindTexture(
        QPixmap( QString("./res/sidekb.png") ),
        GL_TEXTURE_2D
    );
    this->textures->kingTextureWhite[0] = bindTexture(
        QPixmap( QString("./res/sidek.png") ),
        GL_TEXTURE_2D
    );
  }

  /// pick Object at given mouse coordinates
  /**
      \param p mouse coordinates
      \returns Object* to object under mouse cursor
  */
  Object* GLWidget::pickObject(QPoint p){
    // get resolution from settings
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    // get color information from frame buffer
    unsigned char pixel[3];
    this->scene->display(true);
    // Important: gl (0,0) is bottom left but window coords (0,0) are top left -> have to subtract y from height
    glReadPixels(p.x(), viewport[3] - p.y(), 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
    size_t id = Color(pixel[0], pixel[1], pixel[2]).id();
    return Object::objectList.get(id);
  }

  void GLWidget::initializeGL() {
    glEnable(GL_DEPTH_TEST);
    const Color& bgColor = Color::GREY20;
    glClearColor(  bgColor.r
                 , bgColor.g
                 , bgColor.b
                 , 0.0f
    );
    // ...and tell the object list that our background color
    // does not correspond to any kind of object
    Object::objectList.nullId = bgColor.id();
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
    this->scene->display();
  }

  void GLWidget::mousePressEvent(QMouseEvent *event) {
    Object* obj;
    if (event->button() == Qt::LeftButton){
      // pick object
      obj = this->pickObject( event->pos() );
    } else if (event->button() == Qt::RightButton){
      // save mouse position for scene rotation
      this->mousePos = event->pos();
    } else {
      event->ignore();
    }
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
