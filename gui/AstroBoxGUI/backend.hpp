#ifndef BACKEND_HPP
#define BACKEND_HPP

#include <QObject>
#include <qqml.h>
#include <QString>

class Backend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString debugText READ debugText WRITE setDebugText NOTIFY debugTextChanged)

private:
    QString _debugText = "hello world";

public:
    explicit Backend(QObject *parent = nullptr);

    const QString &debugText() const;
    void setDebugText(const QString &newDebugText);

signals:

    void debugTextChanged();
};

#endif // BACKEND_HPP
