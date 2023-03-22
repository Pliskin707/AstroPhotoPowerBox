#ifndef __SWITCHER_HPP__
#define __SWITCHER_HPP__

#include <Arduino.h>
#include <non_copy_class.hpp>

#include "config.hpp"
#include "sensors/power/psens.hpp"
#include "battery/battery.hpp"

#define SWITCHER_PIN_CONSUMERS  (8)     // SD1
#define SWITCHER_PIN_MOUNT      (7)     // SD0
#define SWITCHER_PIN_CHARGER    (11)    // CMD
#define SWITCHER_PIN_CAMERA     (12)    // D6

#define SWITCHER_PIN_HEATER1    (10)    // SD3
#define SWITCHER_PIN_HEATER2    (9)     // SD2
#define SWITCHER_PIN_BTNLED     (0)     // D3
#define SWITCHER_PIN_BUTTON     (13)    // D7

namespace switcher
{

typedef struct
{
    uint32_t lastButtonPressTime;
    uint32_t lastButtonReleaseTime;
    uint32_t numPressesSinceStart;
    bool pressed;
} buttonInfo;

void setup (void);
void loop (void);
void setChargeLimit (const float startSoC, const float stopSoC);

buttonInfo getButtonInfo (void);
void setConsumers (const bool on);
void setMount (const bool on);
void setCamera (const bool on);
void setCharger (const bool on);
void setHeater (const uint_fast8_t heaterNr, const float powerLimit);
bool getConsumers (void);
bool getMount (void);
bool getCamera (void);
bool getCharger (void);
float getHeater (const uint_fast8_t heaterNr);    // returns heater setting in percent

}

#endif