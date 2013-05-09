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

void Config::setAiDepth(size_t aiDepth){
  this->setValue("game/aiDepth", (unsigned int) aiDepth);
}

size_t Config::aiDepth(){
  return (size_t) this->value("game/aidepth").toUInt();
}

KBX::PlayMode Config::playMode(){
  QString val = this->value("game/playMode").toString();
  if (val == "HUMAN_AI"){
    return KBX::HUMAN_AI;
  } else if(val == "AI_HUMAN"){
    return KBX::AI_HUMAN;
  } else if(val == "HUMAN_HUMAN"){
    return KBX::HUMAN_HUMAN;
  } else {
    return KBX::HUMAN_AI;
  }
}

void Config::setPlayMode(KBX::PlayMode mode){
  QString setting;
  switch(mode){
    case KBX::HUMAN_AI:
      setting = QString("HUMAN_AI");
      break;
    case KBX::AI_HUMAN:
      setting = QString("AI_HUMAN");
      break;
    case KBX::HUMAN_HUMAN:
      setting = QString("HUMAN_HUMAN");
      break;
    default:
      setting = QString("HUMAN_AI");
  }
  this->setValue("game/playMode", setting);
}
