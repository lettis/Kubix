
#include "config.hpp"
#include "new_game_dialog.hpp"

NewGameDialog::NewGameDialog(QWidget *parent)
    : QDialog(parent),
      _mode(KBX::HUMAN_AI){
  this->_ui.setupUi(this);
  // load config from previous session
  Config c;
  this->_ui.aiDepth->setValue(c.value("game/aidepth").toInt());
  this->_ui.playMode->setCurrentIndex(c.value("game/mode").toInt());
  QObject::connect(this, SIGNAL(newGame()), this->parent(), SLOT(startNewGame()));
}

void NewGameDialog::accept(){
  //TODO: setup play mode correctly from combobox
  Config c;
  // save game settings
  c.setValue("game/aidepth", this->_ui.aiDepth->value());
  c.setValue("game/mode", this->_ui.playMode->currentIndex());
  // inform main window about changed settings
  //QObject::connect(this  &c, SIGNAL(destroyed()), this->parent(), SLOT(reloadSettings()));
  //emit newGame(c);
  QDialog::accept();
}
