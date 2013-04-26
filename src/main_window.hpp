/*
 * mainWindow.hpp
 *
 *  Created on: Apr 26, 2013
 *      Author: flosit
 */

#ifndef MAINWINDOW_HPP_
#define MAINWINDOW_HPP_

#include "ui_mainWindow.h"
#include "ui_aboutDialog.h"

class MainWindow: public QMainWindow {
  Q_OBJECT

  public:
    MainWindow(QWidget *parent = 0);

  public slots:
    void showAboutDialog();

  private:
    Ui::MainWindow ui;
};

#endif /* MAINWINDOW_HPP_ */
