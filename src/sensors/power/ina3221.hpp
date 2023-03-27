#ifndef __INA3221_HPP__
#define __INA3221_HPP__

#include <Wire.h>
#include "non_copy_class.hpp"
#include "ina3221_types.hpp"

#include "projutils/projutils.hpp"

// #define INA3221_DEBUG

namespace ina3221
{

class sensor : private nonCopyable
{
    private:
        uint8_t _addr;
        TwoWire * const _wire;
        value _voltages[INA3221_NUM_CH] = {0.0f};
        value _currents[INA3221_NUM_CH] = {0.0f};
        adcCorrection _vAdcCorrection[INA3221_NUM_CH];
        adcCorrection _iAdcCorrection[INA3221_NUM_CH];


        uint32_t _minReadDelay      = 10;
        uint16_t _configRegister    = 0x7007 | 
                                        (e_ina3221_avg_4 << 9)       |  // average 4 samples
                                        (e_ina3221_conv_1100us << 6) |  // bus voltage conversion time
                                        (e_ina3221_conv_1100us << 3);   // shunt voltage conversion time

        void _prepareWire (void);
        bool _setRegisterPointer (const uint8_t regAddr);
        bool _read16 (uint16_t &data, const uint8_t addr);
        bool _write16 (const uint16_t data, const uint8_t addr);
        bool _reset (void);
        void _readValue (value &dest, const uint8_t addr, const adcCorrection &correction);

    public:
        explicit sensor (TwoWire * const wire = &Wire);

        bool setup (const uint8_t addr);

        void setReadDelay (const uint32_t readDelay) {_minReadDelay = readDelay;};   // no attempt to read new values from the sensor if this duration did not yet elapse since the last read
        const value& getVoltage (const uint_fast8_t channelIndex);
        const value& getCurrent (const uint_fast8_t channelIndex);
        void setVoltageCorrection (const uint_fast8_t channelIndex, const float factor, const float offset);
        void setCurrentCorrection (const uint_fast8_t channelIndex, const float factor, const float offset);
        void powerDown (void);  // disables measurement. The device will automatically be woken when new values are requested

        static float calcCurrentFactorFromShunt (const float shuntOhm);
};

}

#endif