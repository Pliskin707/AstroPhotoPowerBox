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
    const auto& btn = switcher::getButtonInfo();
    float current = powersensors.getCurrent(e_psens_ch1_battery);
    float voltage = powersensors.getVoltage(e_psens_ch1_battery);
    e_psens_channel e_channel;

    txContent tx;
    consumerInfo * conInfo;

    tx.timestamp = millis();
    tx.pressCount = btn.numPressesSinceStart;
    tx.buttonPressed = btn.pressed;
    tx.shutdownPC = (switcher::getConsumersState() == switcher::consumersState::e_consumers_on_wait4pc_shutdown);
    tx.wifiStrength = WiFi.RSSI();

    tx.batteryInfo.quality = battery.getSoCgood() ? good : medium;
    tx.batteryInfo.SoC = battery.getSoC();
    tx.batteryInfo.energy = battery.getEnergyRemaining();
    tx.batteryInfo.voltage = voltage;
    tx.batteryInfo.current = current;
    tx.batteryInfo.chargeSecondsRemainingToFullSoC = 0;
    tx.batteryInfo.chargeSecondsRemainingToBulkSoC = 0;
    tx.batteryInfo.dischargeSecondsRemainingToEmpty = 0;
    tx.batteryInfo.dischargeSecondsRemainingToReserved = 0;

    tx.dewInfo.quality = bad;
    tx.dewInfo.relHumidity = 0;
    tx.dewInfo.temperature = 0;

    conInfo = &tx.chargerInfo;
    conInfo->isPowered  = switcher::getCharger();
    conInfo->isActive   = (current > 0.1f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_psens_ch1_battery);
    conInfo->avgPower   = powersensors.getAvgPower(e_psens_ch1_battery);

    conInfo = &tx.pcInfo;
    e_channel = e_psens_ch4_pc;
    current = powersensors.getCurrent(e_channel);
    voltage = powersensors.getVoltage(e_channel);
    conInfo->isPowered  = switcher::getConsumers();
    conInfo->isActive   = (current > 0.4f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_channel);
    conInfo->avgPower   = powersensors.getAvgPower(e_channel);

    conInfo = &tx.mountInfo;
    e_channel = e_psens_ch5_mount;
    current = powersensors.getCurrent(e_channel);
    voltage = powersensors.getVoltage(e_channel);
    conInfo->isPowered  = switcher::getMount();
    conInfo->isActive   = (current > 0.4f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_channel);
    conInfo->avgPower   = powersensors.getAvgPower(e_channel);

    conInfo = &tx.cameraInfo;
    e_channel = e_psens_ch6_imaging_cam;
    current = powersensors.getCurrent(e_channel);
    voltage = powersensors.getVoltage(e_channel);
    conInfo->isPowered  = switcher::getCamera();
    conInfo->isActive   = (current > 0.2f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_channel);
    conInfo->avgPower   = powersensors.getAvgPower(e_channel);

    conInfo = &tx.heater1Info;
    e_channel = e_psens_ch2_dew_heater_1;
    current = powersensors.getCurrent(e_channel);
    voltage = powersensors.getVoltage(e_channel);
    conInfo->isPowered  = (switcher::getHeater(0) > 0.0f);
    conInfo->isActive   = (current > 0.2f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_channel);
    conInfo->avgPower   = powersensors.getAvgPower(e_channel);

    conInfo = &tx.heater1Info;
    e_channel = e_psens_ch3_dew_heater_2;
    current = powersensors.getCurrent(e_channel);
    voltage = powersensors.getVoltage(e_channel);
    conInfo->isPowered  = (switcher::getHeater(1) > 0.0f);
    conInfo->isActive   = (current > 0.2f);
    conInfo->voltage    = voltage;
    conInfo->current    = current;
    conInfo->power      = powersensors.getPower(e_channel);
    conInfo->avgPower   = powersensors.getAvgPower(e_channel);

    _doc.set(tx);

    // _udp.beginPacket(_RemoteAddress.IP, _RemoteAddress.port);
    _udp.beginPacket(WiFi.broadcastIP(), UDP_PORT);
    serializeJson(_doc, _udp);
    _udp.endPacket();
}

void communication::setup (void)
{
    _configureService();
    _udp.begin(UDP_PORT - 1);
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

   const auto sysTime = millis();
   if ((sysTime - _lastTxTime) > 1000)
   {
        _lastTxTime = sysTime;
        _send();
   }
}

uint32_t communication::getMsSinceLastRx(void) const
{
    return millis() - _lastRxTime;
}