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
#ifndef GUI_OPENGL__HPP
#define GUI_OPENGL__HPP

#include <QtOpenGL/QGLWidget>
#include <QPoint>
#include <QTimer>
#include <QWheelEvent>

#include "models.hpp"

namespace KBX {
  class GLWidget : public QGLWidget {
      Q_OBJECT // must include this if you use Qt signals/slots
  public:
    Color bgColor;

    void changed();
    void setAutoUpdate(bool newAutoUpdate);

  private:
    Scene* scene;
    QPoint mousePos;
    QTimer* updateTimer;
    
    bool autoRefresh;
    bool autoUpdate;

    int nBuffers;
    // bfChange keeps track of the changes of the scene
    // bfChange  > 0: perform a redraw for the next bfChange frames
    // bfChange == 0: do not perform redraws
    // bfChange  < 0: permanently perform redraws
    int bfChange;
    void updated();
    bool needUpdate();

    Object* pickObject(QPoint p);

    bool relativeMarking;
    
    Logger log;
    
  protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

  protected slots:
    void newGame();
    void setAutoRefresh(bool newAutoRefresh);
    void setRelativeMarking(bool newRelativeMarking);

  public:
    GLWidget(QWidget *parent = NULL);
    ~GLWidget() {}
    void initializeGUI();
  };

} // end namespace KBX

#endif
