#include "declarativecontextproperty.h"
#include <QtQml>
#include "contextproperty.h"

#define URI "Crowd.SystemState"
#define MAJOR 1
#define MINOR 0


DeclarativePlugin::DeclarativePlugin(QObject *parent) :
  QQmlExtensionPlugin(parent) {

}

void DeclarativePlugin::registerTypes(const char *uri) {
  Q_UNUSED(uri);

  Q_ASSERT(uri == QLatin1String(URI));

  qmlRegisterType<DeclarativeContextProperty>(URI, MAJOR, MINOR, "ContextProperty");
}

void DeclarativePlugin::initializeEngine(QQmlEngine *engine, const char *uri) {
  Q_UNUSED(uri);

  Q_ASSERT(uri == QLatin1String(URI));
}

DeclarativeContextProperty::DeclarativeContextProperty(QObject *parent) :
  QObject(parent),
  m_property(nullptr) {

}

DeclarativeContextProperty::~DeclarativeContextProperty() {
  delete m_property;
  m_property = nullptr;
}

QVariant DeclarativeContextProperty::value() const {
  return m_property ? m_property->value() : QVariant();
}

void DeclarativeContextProperty::setValue(const QVariant& value) {
  if (m_property) {
    m_property->setValue(value);
  }
}

QString DeclarativeContextProperty::key() const {
  return m_property ? m_property->key() : QString();
}

void DeclarativeContextProperty::setKey(const QString& key) {
  if (m_property && m_property->key() == key) {
    return;
  }

  delete m_property;
  m_property = new ContextProperty(key, this);
  QObject::connect(m_property, &ContextProperty::valueChanged, this, &DeclarativeContextProperty::valueChanged);
  emit keyChanged();
  emit valueChanged();
}
