#ifndef ESP_COMM_HPP
#define ESP_COMM_HPP

#include <QObject>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkDatagram>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "qtzeroconf/qzeroconf.h"
#include "logger/logger.h"
#include "backend.hpp"

#define UDP_PORT                (5051)
#define SERVICE_NAME            "AstroPowerBox"

class ESP_comm : public QObject
{
    Q_OBJECT

private:
    backend &_backend;
    QHostAddress _espIp = QHostAddress("AstroNode_FCCB1C.local");
    QZeroConf _zeroconf;
    QUdpSocket * _socket = nullptr;
    QHostAddress _espAddr;
    logger _log;

private slots:
    void _ZeroConfServiceSlot (QZeroConfService service);
    void _ReportZCErr (QZeroConf::error_t err);
    void _parseRxData (void);
    void _sockErr (QAbstractSocket::SocketError err);


public:
    explicit ESP_comm(backend &backend, QObject *parent = nullptr);
    ~ESP_comm() {qDebug() << "ESP communication destroyed" << Qt::endl;}

signals:

};

#endif // ESP_COMM_HPP
