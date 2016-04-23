#ifndef CONTEXT_PROPERTY_H
#define CONTEXT_PROPERTY_H

#include <QObject>
#include <QVariant>
#include <QString>

// This interface is preserved as a compatibility for Harmattan and Sailfish OS
// Only some rarely used bits have been removed.

class ContextPropertyPrivate;

class ContextProperty : public QObject {
  Q_OBJECT

public:
  explicit ContextProperty(const QString& key, QObject *parent = 0);

  virtual ~ContextProperty();

  QString key() const;

  QVariant value(const QVariant& def) const;
  QVariant value() const;

  void setValue(const QVariant& value);

  void subscribe() const;
  void unsubscribe() const;

  void waitForSubscription() const;
  void waitForSubscription(bool block) const;

Q_SIGNALS:
  void valueChanged(); ///< Emitted whenever the value of the property changes and the property is subscribed.

private:
  ContextPropertyPrivate *d_ptr;
};

#endif /* CONTEXT_PROPERTY_H */
