#ifndef __PROJUTILS_HPP__
#define __PROJUTILS_HPP__

#include <Arduino.h>
#include <ESP8266WiFi.h>

#ifndef DEBUG_PRINT
#define dprintf(...) ;
#else
#define dprintf(fstr, ...) Serial.printf_P(PSTR(fstr), ##__VA_ARGS__);
#endif

class ClassDeviceName : public String
{
    public:
        ClassDeviceName();
} const DeviceName;

#endif