#ifndef KBX_CONFIG__HPP
#define KBX_CONFIG__HPP

#include <QSettings>

class Config: public QSettings {
  Q_OBJECT

  public:
    Config(QObject * parent = 0);
    // overwrite this to emit signal 'changed'
    void setValue(const QString & key, const QVariant & value);

  signals:
    void changed();
};

#endif
