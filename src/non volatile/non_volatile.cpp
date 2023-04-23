#include "non_volatile.hpp"

non_volatile nvmem;

non_volatile::non_volatile ()
{
    LittleFS.begin();
    File fi = LittleFS.open(NVMEM_FILEPATH, "r");

    if (!fi)
    {
        dprintf("NV-Memory file \"%s\" not found", NVMEM_FILEPATH);
    }
    else
    {
        StaticJsonDocument<256> doc;
        DeserializationError jsonError = deserializeJson(doc, fi);

        if (jsonError)
        {
            dprintf("NV-Memory file deserialization failed with %s", jsonError.c_str());
        }
        else
        {
            _savedStats = _stats = doc.as<batStats>();
        }

        fi.close();
    }
}

bool non_volatile::save(const batStats &stats)
{
    if (_dirty)
    {
        _dirty = false;
        _lastMod = 0;

        StaticJsonDocument<256> doc;

        File fi = LittleFS.open(NVMEM_FILEPATH, "w+");

        // this *should* copy the data to memory, so the file can be altered without affecting the doc (see https://arduinojson.org/v6/issues/altered-input/ )
        // -> the "File" type is being read as "Stream" which causes the JSON Lib to switch to "read-only" mode
        DeserializationError jsonError = deserializeJson(doc, fi);   
        if (jsonError)
        {
            dprintf("User settings deserialization failed with %s", jsonError.c_str());
            doc.clear();
        }

        display.clearDisplay();
        display.setCursor(0, 0);

        // replace the known values
        doc = stats;

        // write the file
        bool success = fi.seek(0, fs::SeekMode::SeekSet);    // is this ".setPosition(0)" ?
        if (success)
        {
            const size_t bytesWritten = serializeJson(doc, fi); 
            dprintf("%u bytes written to \"%s\"", bytesWritten, NVMEM_FILEPATH);

            success = bytesWritten > 0;
            display.print(F("Battery stats\nstored."));
        }
        else
        {
            dprintf("Could not write to \"%s\"", NVMEM_FILEPATH);
            display.print(F("Battery stats\nFAILED!"));
        }
        display.suppressContentTemporary(3000);
        
        fi.close();
        if (success)
            _savedStats = stats;

        return success;
    }

    return false;
}

void non_volatile::setRemainingCharge(const float remainingCharge)
{
    if (remainingCharge != _stats.remainingCharge)
    {
        _modify();
        _stats.remainingCharge = remainingCharge;
    }
}

uint32 non_volatile::getTimeSinceLastModification(void) const
{
    return (_lastMod ? (millis() - _lastMod) : 0);
}

void non_volatile::loop(void)
{
     // TODO also when battery voltage is critical
    if ((getTimeSinceLastModification() > 60000) || 
        (fabsf(_savedStats.remainingCharge - _stats.remainingCharge) > (0.1f * 3600.0f))) // every 100 mAh
    {
        save(_stats);
    }
}
void non_volatile::_modify(void)
{
    _dirty = true;
    _lastMod = millis();
}

bool convertToJson(const batStats& src, JsonVariant dst)
{
    dst[F("total coulomb charged")]     = src.totalCoulombCharged;
    dst[F("total coulomb discharged")]  = src.totalCoulombDischarged;
    dst[F("remaining charge")]          = src.remainingCharge;
    // TODO store last calibration
    // dst[F("last calibration")]          = src.lastCalibration;

    return true;
}

void convertFromJson(JsonVariantConst src, batStats& dst)
{
    JsonVariantConst jVar;
    dst = batStats();   // reset to default

    jVar = src[F("total coulomb charged")];
    if (jVar.is<float>())
        dst.totalCoulombCharged = jVar.as<float>();
    
    jVar = src[F("total coulomb discharged")];
    if (jVar.is<float>())
        dst.totalCoulombDischarged = jVar.as<float>();

    jVar = src[F("remaining charge")];
    if (jVar.is<float>())
        dst.remainingCharge = jVar.as<float>();

    // TODO read last calibration
}
