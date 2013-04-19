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
#ifndef GAME_WIDGET__HPP
#define GAME_WIDGET__HPP

#include <QtOpenGL/QGLWidget>
#include <QPoint>
#include <QTimer>
#include <QWheelEvent>

#include <list>

#include "models.hpp"

namespace KBX {
class GameWidget: public QGLWidget {
    // must include this if you use Qt signals/slots
  Q_OBJECT

  public:
    Color bgColor;

    void changed();
    void setAutoUpdate(bool newAutoUpdate);
    void userSelect(Model* obj);

  private:
    Scene* _scene;
    Game* _game;
    QPoint _mousePos;
    QTimer* _updateTimer;

    bool _autoRefresh;
    bool _autoUpdate;

    int _nBuffers;
    // _bfChange keeps track of the changes of the scene
    // _bfChange  > 0: perform a redraw for the next bfChange frames
    // _bfChange == 0: do not perform redraws
    // _bfChange  < 0: permanently perform redraws
    int _bfChange;
    void _updated();
    bool _needUpdate();

    Model* _pickObject(QPoint p);

    std::list<size_t> _paths;
    void _clearPaths();

    bool _relativeMarking;

    Logger _log;

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
    void save();
    void load();
    void setAutoRefresh(bool newAutoRefresh);
    void setRelativeMarking(bool newRelativeMarking);

  public:
    GameWidget(QWidget *parent = NULL);
    ~GameWidget() {
    }
    void initializeGUI();
};

} // end namespace KBX

#endif
