#ifndef MAINWINDOW_HPP_
#define MAINWINDOW_HPP_

#include <QLabel>

#include "ui_mainWindow.h"
#include "config.hpp"

class MainWindow: public QMainWindow {
    Q_OBJECT
    
  public:
    MainWindow(QWidget *parent = 0);
				   
  public slots:
    void showAboutDialog();
    void showPreferencesDialog();
    void showNewGameDialog();
    void reloadSettings();
    void startNewGame(GameConfig c);
    void setStatus(QString msg);
    
  signals:
    void settingsChanged();
    void newGame(GameConfig c);
    void exitGame();
    
  protected:
    void closeEvent(QCloseEvent* event);
    
  private:
    Ui::MainWindow ui;
    QLabel _statusMsg;
};

#endif
