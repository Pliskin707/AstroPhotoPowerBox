#ifndef __NON_VOLATILE_H__
#define __NON_VOLATILE_H__

#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>
#include <non_copy_class.hpp>
#include "projutils/projutils.hpp"
#include "CRC16.h"
#include "oled/oled.hpp"

#define NVMEM_FILEPATH  "/nvmem.json"

typedef struct
{
    float totalCoulombCharged = 0.0f;
    float totalCoulombDischarged = 0.0f;
    float remainingCharge = 0.0f;
    struct tm lastCalibration;
} batStats;

bool convertToJson(const batStats& src, JsonVariant dst);
void convertFromJson (JsonVariantConst src, batStats& dst);
class non_volatile : private nonCopyable
{
    private:
        bool _dirty = false;
        uint32_t _lastMod = 0;
        uint32_t _lastSave = 0;
        batStats _stats;
        batStats _savedStats;

        void _modify (void);

    public:
        non_volatile();
        bool save (const batStats &stats);
        void setRemainingCharge (const float remainingCharge);
        batStats getStats (void) {return _stats;};
        uint32 getTimeSinceLastModification (void) const;
        void loop (void);
};

extern non_volatile nvmem;  // global instance for non_volatile memory

#endif // __NON_VOLATILE_H__