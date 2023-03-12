#ifndef __ENERGY_SAVER_H__
#define __ENERGY_SAVER_H__

#include <ESP8266WiFi.h>
#include "communication/communication_types.hpp"

class energySaver
{
    private:
        bool _modemSleep = false;
        uint32_t _sleepLimit = 1000;
        uint32_t _wifiConnSince = 0;
        void _setModemSleep (const bool sleep);

    public:
        void setup (void);
        void setSleepLimit (const uint32_t sleepLimit) {_sleepLimit = sleepLimit;};
        uint32_t loop (const uint32_t maxSleepTime = 1000);
        bool getKeepAwake (void) const {return !_modemSleep;};
};

extern energySaver energy; // global energy saver instance

#endif // __ENERGY_SAVER_H__