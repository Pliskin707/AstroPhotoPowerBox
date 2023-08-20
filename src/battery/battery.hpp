#ifndef __BATTERY_HPP__
#define __BATTERY_HPP__

#include "sensors/power/psens.hpp"
#include "switcher/switcher.hpp"
#include "non volatile/non_volatile.hpp"
#include "non_copy_class.hpp"

#define CAPACITY_KWH        (30.0f)


class lifepo4_battery : private nonCopyable
{
    private:
        uint32_t _lastUpdate = 0;
        uint32_t _updateDelay = 200;
        uint32_t _idleCurrentSince = 0;
        float _SoC = 0.0f;                  // [%]
        float _chargeRemaining = 0.0f;      // [C = As]
        int64_t _chargeRemaining_fine = 0;  // [µAs]
        int32_t _prevCurrentMilliAmps = 0;
        bool _initialized = false;
        bool _SoCgood = false;

        void _initFromMemory (void);

    public:
        void loop (void);
        float getSoC (void) const {return _SoC;};                               // [%]
        float getChargeRemaining (void) const {return _chargeRemaining;};       // [C = As]
        float getEnergyRemainingWh (void) const;
        float getCapacityTotal (void) const;                                    // [C = As]
        bool getSoCgood (void) const {return _SoCgood;};
        
};

extern lifepo4_battery battery;

#endif