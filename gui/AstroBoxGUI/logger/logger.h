#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringLiteral>
#include <QLocale>
#include <QDateTime>

class logger : public QObject
{
    Q_OBJECT

private:
    QFile * _file = new QFile("./AstroBoxLog.csv", this);
    QTextStream _stream;
    const QString _separator = QStringLiteral(";");

public:
    explicit logger(QObject *parent = nullptr);
    ~logger();

public slots:
    void append (const uint32_t controllerTime, const float voltage, const float current);

signals:

};

#endif // LOGGER_H
