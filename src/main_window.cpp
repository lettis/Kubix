
#include "main_window.hpp"
#include "preferences_dialog.hpp"

#include "ui_aboutDialog.h"

#include <QSpacerItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
  ui.setupUi(this);
  this->_statusMsg.setText("");
  // empty widget as space-holder
  this->statusBar()->addPermanentWidget(new QWidget(),5);
  this->statusBar()->addPermanentWidget(&(this->_statusMsg), 300);
  //TODO: implement networking and chat to use this...
  ui.chatDock->setVisible(false);
}

void MainWindow::showAboutDialog() {
  Ui::AboutDialog dialog;
  QDialog d;
  dialog.setupUi(&d);
  d.exec();
}

void MainWindow::showPreferencesDialog(){
  Preferences p(this);
  p.exec();
}

void MainWindow::reloadSettings(){
  // just send the signal on to the game widget
  emit this->settingsChanged();
}

void MainWindow::setStatus(bool engineRunning){
  if (engineRunning){
    this->_statusMsg.setText("<b>evaluating next move</b>");
  } else {
    this->_statusMsg.setText("");
  }
}
