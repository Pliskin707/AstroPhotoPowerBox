#include "battery.hpp"

lifepo4_battery battery;    // global instance

void lifepo4_battery::loop (void)
{
    if (millis() >= (_lastUpdate + _updateDelay))
    {
        const float actPower = powersensors.getPower(e_psens_ch1_battery);  // call this first so the (time consuming) TWI communication is handled before the time is calculated
        const float voltage = powersensors.getVoltage(e_psens_ch1_battery);
        const uint32_t sysTime = millis();

        if (_lastUpdate)   // already initialized?
        {
            // calculate the area (= capacity) between the previous and actual power reading with linear interpolation
            const float deltaTime = ((float) (sysTime - _lastUpdate)) / 1000.0f;
            const float powerSum = actPower + _prevPower;
            const float deltaEnergy = (powerSum * deltaTime) / 2.0f;

            // update the known capacity
            _energyRemaining += deltaEnergy;

            // update the state of charge
            float actSoC = _energyRemaining * 100.0f / _energyTotal;
            actSoC = fminf(100.0f, fmaxf(0.0f, actSoC));
            // TODO plausibility with measured battery voltage


            // "brown out" detection
            // TODO if the supply voltage drops / gets critical, store the values in EEPROM

        }

        _lastUpdate  = sysTime;
        _prevPower   = actPower;
        _prevVoltage = voltage;
    }
}