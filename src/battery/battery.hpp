#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

#include "sensors/power/psens.hpp"

class lifepo4_battery
{
    private:
        uint32_t _lastUpdate = 0;
        uint32_t _updateDelay = 200;
        uint32_t _idleCurrentSince = 0;
        float _SoC = 0.0f;                  // [%]
        float _energyRemaining = 0.0f;      // [Ws]
        float _energyTotal = 384.0f * 3.6; // [Ws / 100%]   
        float _prevPower = 0.0f;
        float _prevVoltage = 0.0f;
        bool _stored = false;
        bool _SoCgood = false;

    public:
        void loop (void);
        float getSoC (void) const {return _SoC;};
        float getEnergyRemaining (void) const {return _energyRemaining / 3600.0f;};
        float getEnergyTotal (void) const {return _energyTotal / 3.6f;};
        bool getSoCgood (void) const {return _SoCgood;};
        
};

extern lifepo4_battery battery;

#endif