#include "communication.hpp"

communication comm;

void communication::_configureService(void)
{
    _service = MDNS.addService(DeviceName.c_str(), SERVICE_NAME, "udp", UDP_PORT);
    if (_service != nullptr)
    {
        MDNS.addServiceTxt(_service, "mac", WiFi.macAddress().c_str());
        MDNS.addServiceTxt(_service, "version", VERSION_STRING);
        MDNS.addServiceTxt(_service, "author", "Pliskin707");
    }
}

bool communication::_read(void)
{
    const uint32_t sysTime = millis();

    if (_udp.parsePacket() > 0)
    {
        _jsonError = deserializeJson(_doc, _udp);
        dprintf("JsonError: %s\n", _jsonError.c_str());

        if (!_jsonError)
        {
            _RemoteAddress.IP   = _udp.remoteIP();
            _RemoteAddress.port = _udp.remotePort();
            _rxContent          = _doc.as<rxContent>();
            _lastRxTime         = sysTime;
            return true;
        }
    }

    return false;
}

void communication::_send (void)
{
    /* TODO
    txContent tx =
    {
        .
    };

    _doc.clear();
    if (!buttonStats.firstPressTime)
        tx.timestamp = millis();

    _doc.set(tx);
    _udp.beginPacket(_RemoteAddress.IP, _RemoteAddress.port);
    serializeJson(_doc, _udp);
    _udp.endPacket();
    */
}

void communication::setup (void)
{
    _configureService();
    _udp.begin(UDP_PORT);
}

void communication::loop(void)
{
    _hasNewData = _read();
}

bool communication::hasNewData (void)
{
    return _hasNewData;
}

void communication::transmit (void)
{
    /* TODO
    const auto buttonStats = BuzzerButton::pressCount();
    if (_hasNewData || buttonStats.pressCount)
        _send(buttonStats);
    */
}

uint32_t communication::getMsSinceLastRx(void) const
{
    return millis() - _lastRxTime;
}