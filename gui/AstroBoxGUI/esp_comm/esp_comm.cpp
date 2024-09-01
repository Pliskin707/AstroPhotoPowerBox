#include "esp_comm.hpp"

ESP_comm::ESP_comm(Backend &backend, QObject *parent) : QObject(parent), _backend(backend)
{
    qDebug() << "ESP communication created" << Qt::endl;

    connect(&_zeroconf, &QZeroConf::error, this, &ESP_comm::_ReportZCErr);
    connect(&_zeroconf, &QZeroConf::serviceAdded, this, &ESP_comm::_ZeroConfServiceSlot);

    // request the IP of the ESP
    QString service = QString("_%1._udp").arg(SERVICE_NAME);
    _zeroconf.startBrowser(service);

    // try connecting to static address
    if (!_socket)
    {
        _socket = new QUdpSocket(this);
        if (!_socket)
            qCritical() << "Socket creation failed!" << Qt::endl;
        else
        {
            connect(_socket, &QUdpSocket::readyRead, this, &ESP_comm::_parseRxData);
            connect(_socket, &QUdpSocket::errorOccurred, this, &ESP_comm::_sockErr);
            _socket->bind(5051);
            _espAddr = QHostAddress(QStringLiteral("192.168.1.2"));
        }
    }
}

void ESP_comm::_ReportZCErr(QZeroConf::error_t err)
{
    qWarning() << "ZeroConf Error" << QString("ZeroConf Error %1 reported").arg(err) << Qt::endl;
}

void ESP_comm::_parseRxData()
{
    const auto &rx = _socket->receiveDatagram();
//    qDebug() << "Data received:" << rx.data() << "from" << rx.senderAddress() << Qt::endl;

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
            qWarning() << "Parsing failed" << Qt::endl;
        }
        else
        {
            _backend.setDebugText(doc.toJson());

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

            // auto suspend PC
            if (jObj["shutdown PC"].toBool() && isAstroMiniPc())
            {
                //qDebug() << "Test Shutdown <<<<<<<<" << Qt::endl;
                QProcess::startDetached("shutdown /h /t 10");
            }

        }
    }
}

void ESP_comm::_sockErr(QAbstractSocket::SocketError err)
{
    qWarning() << "Socket error occurred:" << err << Qt::endl;
}

void ESP_comm::_ZeroConfServiceSlot(QZeroConfService service)
{
    qDebug() << "New ZeroConf service" << service->type() << "/" << service->host() << "discovered" << Qt::endl << service << Qt::endl << Qt::endl;

    if (!_socket)
    {
        _socket = new QUdpSocket(this);
        if (!_socket)
            qCritical() << "Socket creation failed!" << Qt::endl;
        else
        {
            connect(_socket, &QUdpSocket::readyRead, this, &ESP_comm::_parseRxData);
            connect(_socket, &QUdpSocket::errorOccurred, this, &ESP_comm::_sockErr);
            _socket->bind(service->port());
            _espAddr = service->ip();
        }
    }
}
