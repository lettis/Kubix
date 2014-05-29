#ifndef PREFERENCES_HPP_
#define PREFERENCES_HPP_

#include "ui_preferencesDialog.h"
#include "engine.hpp"

class Preferences: public QDialog {
  Q_OBJECT

  public:
    Preferences(QWidget *parent = 0);

  public slots:
    void accept();

  private:
    Ui::PreferencesDialog _ui;
    KBX::PlayMode _mode;
};

#endif
