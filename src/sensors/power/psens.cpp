#include "psens.hpp"

psens powersensors;

void psens::setup (TwoWire * wire, const uint8_t addr1, const uint8_t addr2)
{
    const uint8_t addresses[] = {addr1, addr2};
    const float correctionFactor12mOhm = ina3221::sensor::calcCurrentFactorFromShunt(0.012f);
    uint_fast8_t ii = 0;

    for (auto &sensor : _sensors)
    {
        sensor = std::shared_ptr<ina3221::sensor>(new ina3221::sensor(wire));
        sensor->setup(addresses[ii++]);
        sensor->setCurrentCorrection(0, correctionFactor12mOhm, 0.0f);
    }
}

void psens::loop (void)
{
    const uint32_t sysTime = millis();
    uint_fast8_t totalChannel = 0;

    if (sysTime >= _nextUpdate)
    {
        _nextUpdate = sysTime + _updateDelay;

        for (auto &sensor : _sensors)
        {
            for (uint_fast8_t sensChannel = 0; (sensChannel < INA3221_NUM_CH) && (totalChannel < e_psens_num_channels); sensChannel++, totalChannel++)
            {
                _powers[totalChannel] = sensor->getVoltage(sensChannel).value * sensor->getCurrent(sensChannel).value;
            }
        }
    }
}

std::shared_ptr<ina3221::sensor> psens::getSensor(uint_fast8_t sensorIndex)
{
    if (sensorIndex > (sizeof(_sensors) / sizeof(_sensors[0])))
        return nullptr;

    return _sensors[sensorIndex];
}

std::shared_ptr<ina3221::sensor> psens::_getSensorAndChannel (const e_psens_channel channel, uint_fast8_t * sensChannel)
{
    div_t qr = div(static_cast<uint8_t>(channel), INA3221_NUM_CH);
    if (sensChannel)
        *sensChannel = qr.rem;

    return _sensors[qr.quot];
}

float psens::getVoltage(const e_psens_channel channel)
{
    uint_fast8_t sensChannel = 0;
    return _getSensorAndChannel(channel, &sensChannel)->getVoltage(sensChannel).value;
}

float psens::getCurrent(const e_psens_channel channel)
{
    uint_fast8_t sensChannel = 0;
    return _getSensorAndChannel(channel, &sensChannel)->getCurrent(sensChannel).value;
}

float psens::getPower(const e_psens_channel channel)
{
    return _powers[channel];
}