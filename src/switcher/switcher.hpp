#ifndef __SWITCHER_HPP__
#define __SWITCHER_HPP__

#include <Arduino.h>
#include <non_copy_class.hpp>

#include "config.hpp"
#include "sensors/power/psens.hpp"
#include "battery/battery.hpp"

#define SWITCHER_PIN_MOUNT      (10)    // SD3 (Caution: this pin can only be used in DIO mode, see https://nodemcu.readthedocs.io/en/1.5.4.1-final/hardware-faq/#whats-the-different-between-dio-and-qio-mode)
#define SWITCHER_PIN_CONSUMERS  (12)    // D6
#define SWITCHER_PIN_CHARGER    (13)    // D7
#define SWITCHER_PIN_CAMERA     (14)    // D8 (boot fails if pulled high, therefore only connected to a BJT base via 10k resistor)

#define SWITCHER_PIN_HEATER1    (0)     // D3 (internal pull up, boot fails if pulled low, therefore only connected to a MOSFET gate via 2k + 10k resistor)
#define SWITCHER_PIN_HEATER2    (2)     // D4 (same as D3)
#define SWITCHER_PIN_BTNLED     (1)     // TX (debug serial output at boot, may cause flickering of the LED but should be ok nonetheless; boot fails if pulled low, therefore only connected to a BJT base via 10k resistor)
#define SWITCHER_PIN_BUTTON     (3)     // RX (high at boot)

namespace switcher
{

typedef enum 
{
        e_locked_wait4release,
        e_locked_idle, 
        e_unlocking_hold, 
        e_unlocking_wait4release, 
        e_unlocking_release_delay,
        e_unlocking_wait4ack,
        e_unlocked_idle
} keyLockState;

typedef enum 
{
    e_consumers_off_idle,
    e_consumers_off_prepare_activation,
    e_consumers_on_subs_off,
    e_consumers_on_subs_activation,
    e_consumers_on_idle,
    e_consumers_on_prepare_power_down,
    e_consumers_on_wait4pc_shutdown
} consumersState;

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