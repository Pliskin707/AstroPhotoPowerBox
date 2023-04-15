#include "esp_comm.hpp"

ESP_comm::ESP_comm(QObject *parent) : QObject(parent)
{
    qDebug() << "ESP communication created" << endl;

    connect(&_zeroconf, &QZeroConf::error, this, &ESP_comm::_ReportZCErr);
    connect(&_zeroconf, &QZeroConf::serviceAdded, this, &ESP_comm::_ZeroConfServiceSlot);

    // request the IP of the ESP
    QString service = QString("_%1._udp").arg(SERVICE_NAME);
    _zeroconf.startBrowser(service);
}

void ESP_comm::_ReportZCErr(QZeroConf::error_t err)
{
    qWarning() << "ZeroConf Error" << QString("ZeroConf Error %1 reported").arg(err) << endl;
}

void ESP_comm::_parseRxData()
{
    const auto &rx = _socket->receiveDatagram();
//    qDebug() << "Data received:" << rx.data() << "from" << rx.senderAddress() << endl;

    if (rx.senderAddress().isEqual(_espAddr))
    {
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(rx.data(), &err);
        QJsonObject jObj = doc.object();
        QJsonValue val;

        uint32_t controllerTime = 0;
        float batVoltage = 0;
        float batCurrent = 0;
        bool logBat = true;

        if ((err.error != QJsonParseError::NoError) || doc.isNull())
        {
            qWarning() << "Parsing failed" << endl;
        }
        else
        {
            val = jObj["time"];
            if (val.isDouble())
                controllerTime = val.toInt();
            else
                logBat = false;

            val = jObj["battery info"].toObject()["voltage"];
            if (val.isDouble())
            {
                batVoltage = val.toDouble();
                logBat &= (batVoltage >= 1);    // plausibility
            }
            else
                logBat = false;

            val = jObj["battery info"].toObject()["current"];
            if (val.isDouble())
                batCurrent = val.toDouble();
            else
                logBat = false;

            if (logBat)
                _log.append(controllerTime, batVoltage, batCurrent);
        }
    }
}

void ESP_comm::_sockErr(QAbstractSocket::SocketError err)
{
    qWarning() << "Socket error occurred:" << err << endl;
}

void ESP_comm::_ZeroConfServiceSlot(QZeroConfService service)
{
    qDebug() << "New ZeroConf service" << service->type() << "/" << service->host() << "discovered" << endl << service << endl << endl;

    if (!_socket)
    {
        _socket = new QUdpSocket(this);
        if (!_socket)
            qCritical() << "Socket creation failed!" << endl;
        else
        {
            connect(_socket, &QUdpSocket::readyRead, this, &ESP_comm::_parseRxData);
            connect(_socket, QOverload<QAbstractSocket::SocketError>::of(&QUdpSocket::error), this, &ESP_comm::_sockErr);
            _socket->bind(service->port());
            _espAddr = service->ip();
        }
    }
}
