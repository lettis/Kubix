#include "config.hpp"

Config::Config(QObject * parent)
    // call parent constructor with predefined organization/application strings
    // (setting these to some value is necessary)
    : QSettings("lettis", "Kubix", parent) {
}

void Config::setValue(const QString & key, const QVariant & value) {
  // call parent setValue
  QSettings::setValue(key, value);
  // and emit 'changed' signal afterwards
  emit this->changed();
}
