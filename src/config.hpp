#ifndef KBX_CONFIG__HPP
#define KBX_CONFIG__HPP

#include <QSettings>

#include "engine.hpp"

class Config: public QSettings {
  Q_OBJECT

  public:
    Config(QObject * parent = 0);
    // overwrite this to emit signal 'changed'
    void setValue(const QString & key, const QVariant & value);

    void setAiDepth(size_t aiDepth);
    size_t aiDepth();

    void setPlayMode(KBX::PlayMode mode);
    KBX::PlayMode playMode();

  signals:
    void changed();
};

#endif
