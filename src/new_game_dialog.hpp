#ifndef NEW_GAME_HPP_
#define NEW_GAME_HPP_

#include "ui_newGameDialog.h"
#include "engine.hpp"
#include "config.hpp"

class NewGameDialog: public QDialog {
  Q_OBJECT

  public:
    NewGameDialog(QWidget *parent = 0);

  public slots:
    void accept();

  signals:
    void newGame(Config c);

  private:
    Ui::NewGameDialog _ui;
    KBX::PlayMode _mode;
};

#endif
