#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

#include "sensors/power/psens.hpp"
#include "switcher/switcher.hpp"
#include "non volatile/non_volatile.hpp"

class lifepo4_battery
{
    private:
        uint32_t _lastUpdate = 0;
        uint32_t _updateDelay = 200;
        uint32_t _idleCurrentSince = 0;
        float _SoC = 0.0f;                  // [%]
        float _chargeRemaining = 0.0f;      // [C = As]
        float _chargeTotal = 30.0f * 36.0f; // [As / 100%]   
        float _prevCurrent = 0.0f;
        float _prevVoltage = 0.0f;
        bool _initialized = false;
        bool _SoCgood = false;

        void _initFromMemory (void);

    public:
        void loop (void);
        float getSoC (void) const {return _SoC;};                               // [%]
        float getChargeRemaining (void) const {return _chargeRemaining;};       // [C = As]
        float getEnergyRemainingWh (void) const;
        float getCapacityTotal (void) const {return _chargeTotal * 100.0f;};    // [C = As]
        bool getSoCgood (void) const {return _SoCgood;};
        
};

extern lifepo4_battery battery;

#endif