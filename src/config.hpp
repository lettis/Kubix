#ifndef KBX_CONFIG__HPP
#define KBX_CONFIG__HPP

#include <QSettings>

#include "global.hpp"

class GameConfig {
  protected:
    size_t _aiDepth;
    KBX::Strategy _aiStrategy;
    KBX::PlayMode _playMode;

  public:
    GameConfig();
    GameConfig(const GameConfig& other);
    GameConfig(const GameConfig* other);

    virtual void setAiStrategy(KBX::Strategy s);
    virtual KBX::Strategy getAiStrategy() const;

    virtual void setAiDepth(size_t aiDepth);
    virtual size_t getAiDepth() const;
    
    virtual void setPlayMode(KBX::PlayMode mode);
    virtual KBX::PlayMode getPlayMode() const;

};
  
class Config: public QSettings, public GameConfig {
  Q_OBJECT

  public:
    Config(QObject * parent = 0);
    // overwrite this to emit signal 'changed'
    void setValue(const QString & key, const QVariant & value);

    void setAiDepth(size_t aiDepth) override;
    size_t getAiDepth() const override;

    void setPlayMode(KBX::PlayMode mode) override;
    KBX::PlayMode getPlayMode() const override;

  signals:
    void changed();
};

#endif
