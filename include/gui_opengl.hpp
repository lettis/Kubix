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
#include <QWheelEvent>

#include "models.hpp"

namespace KBX {
  class GLWidget : public QGLWidget {
      Q_OBJECT // must include this if you use Qt signals/slots
  public:
      struct Textures {
        GLuint dieTexturesBlack[6];
        GLuint dieTexturesWhite[6];
        GLuint kingTextureBlack[1];
        GLuint kingTextureWhite[1];
      };
      Textures* textures;

  private:
      void loadTextures();

      Scene* scene;
      QPoint mousePos;

      Object* pickObject(QPoint p);

      Logger log;
  
  protected:
      void initializeGL();
      void resizeGL(int w, int h);
      void paintGL();
      void mousePressEvent(QMouseEvent *event);
      void mouseMoveEvent(QMouseEvent *event);
      void mouseWheelEvent(QWheelEvent *event);
      void keyPressEvent(QKeyEvent *event);
  public:
      GLWidget(QWidget *parent = NULL);
      ~GLWidget() {}
  };

} // end namespace KBX

#endif
