
#include "main_window.hpp"
#include "preferences_dialog.hpp"

#include "ui_aboutDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
  ui.setupUi(this);
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
