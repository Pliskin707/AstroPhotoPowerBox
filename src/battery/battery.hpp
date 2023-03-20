#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

#include "sensors/power/psens.hpp"

class lifepo4_battery
{
    private:
        uint32_t _lastUpdate = 0;
        uint32_t _updateDelay = 10;
        float _SoC = 0.0f;
        float _capacityRemaining = 0.0f;
        float _capacityTotal = 384.0f;
        float _prevPower = 0.0f;

    public:
        void loop (void);
        float getSoC (void) const {return _SoC;};
        float getCapacityRemaining (void) const {return _capacityRemaining;};
        float getCapacityTotal (void) const {return _capacityTotal;};
        
};

extern lifepo4_battery battery;

#endif