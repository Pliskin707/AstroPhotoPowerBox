#ifndef __NON_VOLATILE_H__
#define __NON_VOLATILE_H__

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include <non_copy_class.hpp>
#include "projutils/projutils.hpp"

#define NVMEM_FILEPATH  "/nvmem.json"

typedef struct
{
    uint32_t totalEnergyCharged = 0.0f;
    uint32_t totalEnergyDischarged = 0.0f;
    uint32_t remainingEnergy = 0.0f;
    struct tm lastCalibration;
} batStats;

class non_volatile : private nonCopyable
{
    private:
        bool _dirty = false;
        uint32_t _lastMod = 0;
        batStats _stats;

        void _modify (void);

    public:
        non_volatile();
        bool save (const batStats &stats);
        batStats getStats (void) {return _stats;};
        uint32 getTimeSinceLastModification (void) const;
        void loop (void);
};

extern non_volatile nvmem;  // global instance for non_volatile memory

#endif // __NON_VOLATILE_H__