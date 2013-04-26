#include "main_window.hpp"

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
