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
            //_user = doc["user"] | "";   // empty string as default value
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

        // replace the known values TODO
        //doc["user"] = _user;

        // write the file
        const bool success = fi.seek(0, fs::SeekMode::SeekSet);    // is this ".setPosition(0)" ?
        if (success)
        {
            const size_t bytesWritten = serializeJson(doc, fi); 
            (void) bytesWritten; // suppress "unused variable" warning if no debug print is enabled
            dprintf("%u bytes written to \"%s\"", bytesWritten, NVMEM_FILEPATH);
        }
        else
            dprintf("Could not write to \"%s\"", NVMEM_FILEPATH);
        
        fi.close();
        return success;
    }

    return false;
}

uint32 non_volatile::getTimeSinceLastModification(void) const
{
    return (_lastMod ? (millis() - _lastMod) : 0);
}

void non_volatile::loop(void)
{
    if (getTimeSinceLastModification() > 60000) // TODO also when battery voltage is critical (maybe every 10 seconds then? or every 100 mWh?)
        save(_stats);
}
void non_volatile::_modify(void)
{
    _dirty = true;
    _lastMod = millis();
}
