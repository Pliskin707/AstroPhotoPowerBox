#include "battery.hpp"

lifepo4_battery battery;    // global instance

void lifepo4_battery::loop (void)
{
    if (millis() >= (_lastUpdate + _updateDelay))
    {
        const float actPower = powersensors.getPower(e_psens_ch1_battery);  // call this first so the (time consuming) TWI communication is handled before the time is calculated
        const uint32_t sysTime = millis();

        if (_lastUpdate)   // already initialized?
        {
            // calculate the area (= capacity) between the previous and actual power reading with linear interpolation
            const float deltaTime = (float) (sysTime - _lastUpdate);
            const float powerSum = actPower + _prevPower;
            const float deltaCapacity = (powerSum * deltaTime) / 2.0f;

            // update the known capacity
            _capacityRemaining += deltaCapacity;

            // update the state of charge
            float actSoC = _capacityTotal / _capacityRemaining;
            actSoC = fminf(100.0f, fmaxf(0.0f, actSoC));
            // TODO plausibility with measured battery voltage
        }

        _lastUpdate = sysTime;
        _prevPower = actPower;
    }
}