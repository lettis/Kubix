
#include "config.hpp"
#include "new_game_dialog.hpp"

NewGameDialog::NewGameDialog(QWidget *parent)
    : QDialog(parent),
      _mode(KBX::HUMAN_AI){
  this->_ui.setupUi(this);
  // load config from previous session
  Config c;
  this->_ui.aiDepth->setValue(c.getAiDepth());
  this->_ui.playMode->setCurrentIndex(c.getPlayMode());
  QObject::connect(this, SIGNAL(newGame(GameConfig)), this->parent(), SLOT(startNewGame(GameConfig)));
}

void NewGameDialog::accept(){
  //TODO: setup play mode correctly from combobox
  Config c;
  // inform main window about changed settings
  emit newGame(c);
  QDialog::accept();
}
