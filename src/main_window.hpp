/*
 * mainWindow.hpp
 *
 *  Created on: Apr 26, 2013
 *      Author: flosit
 */

#ifndef MAINWINDOW_HPP_
#define MAINWINDOW_HPP_

#include <QLabel>

#include "ui_mainWindow.h"

class MainWindow: public QMainWindow {
  Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);

  public slots:
    void showAboutDialog();
    void showPreferencesDialog();
    void reloadSettings();
    void setStatus(bool engineRunning);

  signals:
    void settingsChanged();

  private:
    Ui::MainWindow ui;
    QLabel _statusMsg;
};

#endif /* MAINWINDOW_HPP_ */
