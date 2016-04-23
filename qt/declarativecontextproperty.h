#ifndef DECLARATIVE_CONTEXT_PROPERTY_H
#define DECLARATIVE_CONTEXT_PROPERTY_H

#include <QQmlExtensionPlugin>
#include <QVariant>

class ContextProperty;

class DeclarativePlugin : public QQmlExtensionPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
  DeclarativePlugin(QObject *parent = 0);
  void registerTypes(const char *uri);
  void initializeEngine(QQmlEngine *engine, const char *uri);
};

class DeclarativeContextProperty : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
  Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)

public:
  DeclarativeContextProperty(QObject *parent = 0);
  ~DeclarativeContextProperty();

  QVariant value() const;

  QString key() const;
  void setKey(const QString& key);

signals:
  void valueChanged();
  void keyChanged();

private:
  ContextProperty *m_property;
};

#endif /* DECLARATIVE_CONTEXT_PROPERTY_H */
