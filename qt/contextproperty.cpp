#include "contextproperty.h"
#include "connection.h"
#include <QMutex>

class ContextPropertyPrivate {
public:
  ContextPropertyPrivate(const QString& key, ContextProperty *parent) :
    m_parent(parent),
    m_key(key),
    m_conn(nullptr) {

    subscribe();
  }

  ~ContextPropertyPrivate() {
    unsubscribe();
    delete m_conn;
    m_conn = nullptr;
  }

  QString key() const { return m_key; }

  void subscribe();
  void unsubscribe();
  QVariant value(const QVariant& def);
  void setValue(const QVariant& value);

private:
  QVariant m_value;
  QString m_key;
  ContextProperty *m_parent;
  Connection *m_conn;
  QMutex m_lock;
};

void ContextPropertyPrivate::subscribe() {
  if (m_conn) {
    return;
  }

  m_conn = new Connection(m_key.toStdString());

  m_conn->connect([this](const std::string& val) {
      // Called from an arbitrary thread

      QMutexLocker l(&m_lock);
      m_value = QVariant(QString::fromStdString(val));
      QMetaObject::invokeMethod(m_parent, "valueChanged", Qt::QueuedConnection);
    });
}

void ContextPropertyPrivate::unsubscribe() {
  if (m_conn) {
    m_conn->disconnect();
    delete m_conn;
    m_conn = nullptr;
  }
}

QVariant ContextPropertyPrivate::value(const QVariant& def) {
  QMutexLocker l(&m_lock);

  return m_value.isValid() ? m_value : def;
}

void ContextPropertyPrivate::setValue(const QVariant& value) {
  Q_ASSERT(m_conn);
  m_conn->setValue(value.toString().toStdString());
}

ContextProperty::ContextProperty(const QString& key, QObject *parent) :
  QObject(parent),
  d_ptr(new ContextPropertyPrivate(key, this)) {

}

ContextProperty::~ContextProperty() {
  delete d_ptr;
  d_ptr = nullptr;
}

QString ContextProperty::key() const {
  return d_ptr->key();
}

QVariant ContextProperty::value(const QVariant& def) const {
  return d_ptr->value(def);
}

QVariant ContextProperty::value() const {
  return value(QVariant());
}

void ContextProperty::setValue(const QVariant& value) {
  d_ptr->setValue(value);
}

void ContextProperty::subscribe() const {
  return d_ptr->subscribe();
}

void ContextProperty::unsubscribe() const {
  return d_ptr->unsubscribe();
}

void ContextProperty::waitForSubscription() const {
  // Nothing
}

void ContextProperty::waitForSubscription(bool block) const {
  Q_UNUSED(block);
  // Nothing
}
