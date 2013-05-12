
#include "config.hpp"
#include "preferences_dialog.hpp"

Preferences::Preferences(QWidget *parent)
    : QDialog(parent),
      _mode(KBX::HUMAN_AI){
  this->_ui.setupUi(this);
  // load config from previous session
  Config c;
  this->_ui.aiDepth->setValue(c.value("game/aidepth").toInt());
  this->_ui.playMode->setCurrentIndex(c.value("game/mode").toInt());
}

void Preferences::accept(){

  //TODO: setup play mode correctly from combobox

  Config c;
  // save game settings
  c.setValue("game/aidepth", this->_ui.aiDepth->value());
  c.setValue("game/mode", this->_ui.playMode->currentIndex());
  // inform main window about changed settings
  QObject::connect(&c, SIGNAL(destroyed()), this->parent(), SLOT(reloadSettings()));
  QDialog::accept();
}
