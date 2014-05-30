#include "config.hpp"


void GameConfig::setAiDepth(size_t aiDepth){
  this->_aiDepth = aiDepth;
}

size_t GameConfig::getAiDepth() const {
  return this->_aiDepth;
}

KBX::PlayMode GameConfig::getPlayMode() const {
  return this->_playMode;
}

void GameConfig::setPlayMode(KBX::PlayMode mode){
  this->_playMode = mode;
}

void GameConfig::setAiStrategy(KBX::Strategy s){
  this->_aiStrategy = s;
}

KBX::Strategy GameConfig::getAiStrategy() const {
  return this->_aiStrategy;
}
#include <iostream>
GameConfig::GameConfig(const GameConfig& other) :
  _aiDepth(other.getAiDepth()),
  _aiStrategy(other.getAiStrategy()),
  _playMode(other.getPlayMode())
{
}

GameConfig::GameConfig(const GameConfig* other) :
  _aiDepth(other ? other->getAiDepth() : 1),
  _aiStrategy(other ? other->getAiStrategy() : KBX::Strategy()),
  _playMode(other ? other->getPlayMode() : KBX::HUMAN_AI)
{
}

GameConfig::GameConfig() :
  _aiDepth(1),
  _aiStrategy(),
  _playMode(KBX::HUMAN_AI)
{
}

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

size_t Config::getAiDepth() const {
  size_t aiDepth = this->value("game/aidepth").toUInt();
  return aiDepth;
}

KBX::PlayMode Config::getPlayMode() const {
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
