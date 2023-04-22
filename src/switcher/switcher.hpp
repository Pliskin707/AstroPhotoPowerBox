#ifndef __SWITCHER_HPP__
#define __SWITCHER_HPP__

#include <Arduino.h>
#include <non_copy_class.hpp>

#include "config.hpp"
#include "sensors/power/psens.hpp"
#include "battery/battery.hpp"
#include "switcher_types.hpp"

namespace switcher
{

void setup (void);
void loop (void);
void setChargeLimit (const float startSoC, const float stopSoC);

buttonInfo getButtonInfo (void);
void setConsumers (const bool on);
void setMount (const bool on);
void setCamera (const bool on);
void setCharger (const bool on);
void setHeater (const uint_fast8_t heaterNr, const float powerLimit);   // sets the power limit in Watts
bool getConsumers (void);
bool getMount (void);
bool getCamera (void);
bool getCharger (void);
float getHeater (const uint_fast8_t heaterNr);    // returns heater setting in percent
int getButtonLedPwm (void); // for debugging
keyLockState getKeyLockState (void);
consumersState getConsumersState (void);
}

#endif