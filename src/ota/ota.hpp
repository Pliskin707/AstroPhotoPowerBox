#ifndef __OTA_HPP__
#define __OTA_HPP__

#include <ArduinoOTA.h>
#include <LittleFS.h>
#include "oled/oled.hpp"
#include "switcher/switcher.hpp"

namespace pliskin
{

/**
 * @brief This is just a wrapper for the individual ArduinoOTA steps 
 * 
 * Used to tidy up the code a bit
 */
class ota
{
    private:
        ota();  // no need to construct (everything is static)
        ~ota();

    public:
        static void begin (const char * const deviceName, statusDisplay * const pDisplay = nullptr);
        static void handle (void) {ArduinoOTA.handle();};
        static bool isUpdating (void);
};

};

#endif