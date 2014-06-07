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
#include <QTimer>
#include <QWheelEvent>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QMutex>

#include <list>

#include "models.hpp"
#include "config.hpp"

namespace KBX {

// use this function to run as worker
// to evaluate moves in separate thread
Move evaluatorFunc(Game game);


class GameWidget: public QGLWidget {
  Q_OBJECT

  friend class Scene;

  public:
    GameWidget(QWidget *parent = NULL);

    Color bgColor;
    void changed();
    void setAutoUpdate(bool newAutoUpdate);
    void userSelect(Model* obj);
    void setBackgroundColor();

  public slots:
    void setAutoRefresh(bool newAutoRefresh);
    void setRelativeMarking(bool newRelativeMarking);

    void newGame(GameConfig c);
    void save();
    void load();
    void load(std::string ifname);
    void update();
    void setEngineRunning();
    void setEngineFinished();
    void performEvaluatedMove();
    void giveUp();
    void cancelEvaluation();

    void undoLastMove();
    void redoLastMove();

  signals:
    void newStatus(QString msg);

  protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

  private:
    Scene* _scene;
    Game* _game;
    Die* _selectedDie;
    QPoint _mousePos;
    QTimer* _updateTimer;
    bool _autoRefresh;
    bool _autoUpdate;
    bool _allowUndoRedo;
    int _nBuffers;
    // _bfChange keeps track of the changes of the scene
    // _bfChange  > 0: perform a redraw for the next bfChange frames
    // _bfChange == 0: do not perform redraws
    // _bfChange  < 0: permanently perform redraws
    int _bfChange;
    void _updated();
    bool _needUpdate();
    Model* _pickObject(QPoint p);
    std::list< size_t > _paths;
    void _clearDieSelection();
    void _performMove(Move m);
    bool _relativeMarking;
    Logger _log;
    Move _moveToPerform;
    QFuture<Move> _eval;
    QFutureWatcher<Move> _watcher;
    bool _engineMoves();
};

} // end namespace KBX

#endif
