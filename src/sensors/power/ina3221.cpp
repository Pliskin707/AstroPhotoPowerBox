#include "ina3221.hpp"

/*
master = this device
slave = sensor

change register pointer:
- 1. byte = slave address with R/!W low -> ACK from slave
- 2. byte = register address

write:
- 1. & 2. byte = change register pointer
- 3. & 4. byte = data (MSB first)

read:
- 1. & 2. byte = change register pointer
- start condition by master followed by slave address with R/!W high
- MSB transmitted from slave -> ACK from master
- LSB transmitted from slave -> master stops transmission with either of these: {NACK, Start, Stop}
*/

namespace ina3221
{

static const value dummy = {0.0f, 0, false};

sensor::sensor (TwoWire * const wire) : _wire(wire) 
{
    const float defaultShunt = calcCurrentFactorFromShunt(0.1f);    // 100 mOhm
    const float defaultVolt  = 0.008f;                              // 80 mV / step

    for (uint_fast8_t ii = 0; ii < INA3221_NUM_CH; ii++)
    {
        setVoltageCorrection(ii, defaultVolt, 0.0f);
        setCurrentCorrection(ii, defaultShunt, 0.0f);
    }
}

void sensor::_prepareWire (void)
{
    _wire->begin();                 // set this device = master
    _wire->setClock(400000);        // max speed
    _wire->flush();
    while (_wire->available())
        _wire->read();              // flush read buffer
}

bool sensor::_setRegisterPointer (const uint8_t regAddr)
{
    _wire->beginTransmission(_addr);
    const bool success = (_wire->write(regAddr) == 1);

    if (!success)
        _wire->endTransmission();

    return success;
}

bool sensor::_read16 (uint16_t &data, const uint8_t addr)
{
    bool success = _setRegisterPointer(addr);
    uint16_t rx  = 0;

    if (success)
    {
        _wire->endTransmission(false);
        _wire->requestFrom(_addr, static_cast<size_t>(2), true);
        if (_wire->available() == 2)
        {
            rx = _wire->read();
            rx <<= 8;
            rx |= _wire->read();

            success = true;
        }
        _wire->endTransmission();
    }

    data = rx;
    return success;
}

bool sensor::_write16 (const uint16_t data, const uint8_t addr)
{
    bool success = _setRegisterPointer(addr);
    size_t bytesTransmitted = 0;

    if (success)
    {
        _wire->endTransmission(false);
        bytesTransmitted += _wire->write(static_cast<uint8_t>((data >> 8) & 0xFF));
        bytesTransmitted += _wire->write(static_cast<uint8_t>(data & 0xFF));

        success = (bytesTransmitted == 2);
    }

    return success;
}

bool sensor::_reset (void)
{
    bool success = false;

    do
    {
        _prepareWire();

        if (!_write16(_configRegister, 0x00))
            break;

        success = true;
    } while (false);
    
    return success;
}

bool sensor::setup (const uint8_t addr)
{
    bool success = false;

    _wire->begin();
    _wire->setTimeout(1000 * 500);  // timeout in microseconds
    success = _reset();

    return success;
}

const value& sensor::getVoltage (const uint_fast8_t channelIndex)
{
    if (channelIndex >= INA3221_NUM_CH)
        return dummy;

    const uint8_t regAddr = 0x02 + (2 * channelIndex);
    value &val = _voltages[channelIndex];

    _readValue(val, regAddr, _vAdcCorrection[channelIndex]);
    return val;
}

const value& sensor::getCurrent (const uint_fast8_t channelIndex)
{
    if (channelIndex >= INA3221_NUM_CH)
        return dummy;

    const uint8_t regAddr = 0x01 + (2 * channelIndex);
    value &val = _currents[channelIndex];

    _readValue(val, regAddr, _iAdcCorrection[channelIndex]);
    return val;
}

void sensor::_readValue (value &dest, const uint8_t addr, const adcCorrection &correction)
{
    const uint32_t sysTime = millis();
    uint16_t rawData = 0;
    bool valid = false;

    do
    {
        if ((sysTime < (dest.time + _minReadDelay)) && dest.valid)
        {
            valid = true;
            break;  // do not read updated values yet
        }

        // read new data
        if (!_read16(rawData, addr))
            break;

        // parse data
        dest.time = sysTime;
        dest.adc = (rawData >> 3);

        // negative?
        if (rawData & (1 << 15))
            dest.adc |= 0xF000;  // two's complement

        dest.value = fmaf(dest.adc, correction.factor, correction.offset);
        valid = true;
    } while (false);

    dest.valid = valid;
}

void sensor::setVoltageCorrection (const uint_fast8_t channelIndex, const float factor, const float offset)
{
    if (channelIndex >= INA3221_NUM_CH)
        return;

    auto &correction  = _vAdcCorrection[channelIndex];
    correction.factor = factor;
    correction.offset = offset;
}

void sensor::setCurrentCorrection (const uint_fast8_t channelIndex, const float factor, const float offset)
{
    if (channelIndex >= INA3221_NUM_CH)
        return;

    auto &correction  = _iAdcCorrection[channelIndex];
    correction.factor = factor;
    correction.offset = offset;
}

void sensor::powerDown (void)
{
    const uint16_t prevConfigReg = _configRegister;
    _configRegister &= ~0x0007; // power down mode
    _reset();
    _configRegister = prevConfigReg;
}

float sensor::calcCurrentFactorFromShunt (const float shuntOhm)
{
    float val = 0.0f;
    const float shuntVoltagePerAdcStep = 0.00004f;  // 40 µV / step
    if (!std::isnan(shuntOhm))
    {
        // I = U/R
        val = shuntVoltagePerAdcStep / shuntOhm;
    }

    return val;
}

}