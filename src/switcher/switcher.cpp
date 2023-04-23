#include "switcher.hpp"

namespace switcher 
{

static volatile buttonInfo _btnInfo = {{0}};
static float _chargeLimitSoC[2] = {100.0f, 95.0f};  // stop and restart SoC
static int _heaterPwmValues[2] = {0};
static int _ledPwmValue = 0;
static keyLockState _keyLockState = e_locked_idle;
static uint32_t _unlockDuration = 10000;
static consumersState _consumersState = e_consumers_off_idle;
static bool _consumerPowerRequest = false;

static void IRAM_ATTR _buttonISR (void)
{
    const bool state        = digitalRead(SWITCHER_PIN_BUTTON);
    const uint32_t sysTime  = millis();

    if (state)
        _btnInfo.lastButtonReleaseTime = sysTime;
    else
    {
        _btnInfo.lastButtonPressTime = sysTime;
        _btnInfo.numPressesSinceStart++;
    }

    _btnInfo.pressed = !state;
}

static void _setLedPwm (const uint16_t pwmVal)
{

    #ifdef DEBUG_PRINT
    (void) pwmVal;  // don't do anything since this is the Serial Tx pin
    #else
    if (pwmVal != _ledPwmValue)
    {
        analogWrite(SWITCHER_PIN_BTNLED, pwmVal);
        _ledPwmValue = pwmVal;
    }
    #endif
}

static bool _buttonLoop (void)
{
    const uint32_t sysTime = millis();
    const buttonInfo btnInfo = getButtonInfo();

    switch (_keyLockState)
    {
        case e_locked_wait4release:
        {
            if (!btnInfo.pressed)
                _keyLockState = e_locked_idle;
        }
        break;

        case e_locked_idle:
        {
            _setLedPwm(5);

            // wait for a button press
            if (btnInfo.pressed)
            {
                _keyLockState = e_unlocking_hold;
                dprintf("Button pressed. Starting unlock sequence...\n")
            }
        }
        break;

        case e_unlocking_hold:
        {
            _setLedPwm(511);

            // keep the button presse for about one second
            if (!btnInfo.pressed)
            {
                _keyLockState = e_locked_idle;
                dprintf("Button released too early. Returning to idle state.\n")
            }
            else if ((sysTime - btnInfo.lastButtonPressTime) >= 1000)
            {
                _keyLockState = e_unlocking_wait4release;
                dprintf("Button hold complete. Waiting for release...\n")
            }
        }
        break;

        case e_unlocking_wait4release:
        {
            _setLedPwm(5);

            // wait for release. 
            // if this does not happen within one second the button might be pressed unintentionally (i.e. by some object falling or pressing against the button)
            if (!btnInfo.pressed)
            {
                _keyLockState = e_unlocking_release_delay;
                dprintf("Button released. Waiting for completion delay...\n")
            }
            else if ((sysTime - btnInfo.lastButtonPressTime) >= 2000)
            {
                _keyLockState = e_locked_wait4release;
                dprintf("Button was not released. Returning to initialization.\n")
            }
        }
        break;

        case e_unlocking_release_delay:
        {
            // button must not be pressed during this time
            if (btnInfo.pressed)
            {
                _keyLockState = e_locked_wait4release;
                dprintf("Button pressed too early. Returning to initialization.\n")
            }
            else if ((sysTime - btnInfo.lastButtonReleaseTime) >= 1000)
            {
                _keyLockState = e_unlocking_wait4ack;
                dprintf("Completion delay completed. Waiting for activation...\n")
            }
        }
        break;

        case e_unlocking_wait4ack:
        {
            _setLedPwm(1023);

            // wait for a final button press to acknowledge the unlock procedure
            if (btnInfo.pressed)
            {
                _keyLockState = e_unlocked_idle;
                dprintf("Activation complete. Button will remain unlocked for some time.\n")
            }
            else if ((sysTime - btnInfo.lastButtonReleaseTime) >= 2000)
            {
                _keyLockState = e_locked_idle;
                dprintf("Activation window expired. Returning to idle state.\n")
            }
        }
        break;

        case e_unlocked_idle:
        {
            const int ledPwmVal = btnInfo.pressed ? 1023 : (_consumerPowerRequest ? 255 : 5);
            _setLedPwm(ledPwmVal);

            // auto-lock after a minute
            if (((sysTime - btnInfo.lastButtonReleaseTime) >= _unlockDuration) && 
                ((sysTime - btnInfo.lastButtonPressTime) >= _unlockDuration))
            {
                // no activity within the last minute
                _keyLockState = e_locked_wait4release;
                dprintf("Button locked due to inactivity.\n")
            }
        }
        break;
    }

    static uint32_t prevBtnPressCount = 0;
    if ((btnInfo.numPressesSinceStart != prevBtnPressCount) && 
        (_keyLockState == e_unlocked_idle) &&
        ((sysTime - btnInfo.lastButtonReleaseTime) > 250))  // debounce 
    {
        // toggle the consumers power request
        _consumerPowerRequest ^= true;
        dprintf("User power request is now %s\n", (_consumerPowerRequest ? "ON":"OFF"))
    }

    prevBtnPressCount = btnInfo.numPressesSinceStart;
    return _consumerPowerRequest;
}

static void _consumerPowerLoop (const bool requestedState)
{
    const auto prevState = _consumersState;
    static uint32_t lastStateChange = 0;
    const uint32_t timeSinceStateChange = millis() - lastStateChange;

    switch (_consumersState)
    {
        case e_consumers_off_idle:
        {
            if (requestedState)
                _consumersState = e_consumers_off_prepare_activation;
            else
            {
                // set all relays and components to their least power consuming state
                setConsumers(false);
                setMount(true);
                setCamera(true);
                setHeater(0, 0.0f);
                setHeater(1, 0.0f);
            }
        }
        break;

        case e_consumers_off_prepare_activation:
        {
            // reduce the peak current by switching off all devices first
            setMount(false);
            setCamera(false);
            setHeater(0, 0);
            setHeater(1, 0);

            if (timeSinceStateChange > 200)
                _consumersState = e_consumers_on_subs_off;
        }
        break;

        case e_consumers_on_subs_off:
        {
            setConsumers(true);
            const float voltage = powersensors.getVoltage(e_psens_ch5_mount);
            if ((timeSinceStateChange > 100) && (voltage >= BAT_MIN_TURN_ON_VOLTAGE)) // wait for capacities to load
                _consumersState = e_consumers_on_subs_activation;
            else if (timeSinceStateChange > 3000)
                _consumersState = e_consumers_off_idle;  // TODO generate a warning message (telegram?)
        }
        break;

        case e_consumers_on_subs_activation:
        {
            setCamera(true);

            if (timeSinceStateChange >= 500) 
                setMount(true);

            if (timeSinceStateChange >= 1000) 
                _consumersState = e_consumers_on_idle;
        }
        break;

        case e_consumers_on_idle:
        {
            //! debug
            if (timeSinceStateChange > 60000)
                setHeater(1, 0);
            else
                setHeater(1, 3);

            if (!requestedState)
                _consumersState = e_consumers_on_prepare_power_down;
        }
        break;

        case e_consumers_on_prepare_power_down:
        {
            setMount(false);
            // leave the camera on in case some data transmission is going on

            if (timeSinceStateChange > 200)
                _consumersState = e_consumers_on_wait4pc_shutdown;
        }
        break;

        case e_consumers_on_wait4pc_shutdown:
        {
            if (requestedState)
                _consumersState = e_consumers_on_subs_activation;
            else if ((powersensors.getCurrent(e_psens_ch4_pc) < 0.2) ||  // pc may use up to 57 mA while powered down
                (timeSinceStateChange > 3600000))                   // timeout after one hour (don't make this too short in case the PC is performing updates)
                _consumersState = e_consumers_off_idle;
        }
        break;
    }

    if (_consumersState != prevState)
        lastStateChange = millis();
}

static void _chargeSwitchLoop (void)
{
    static bool wasCharging = false;
    if (powersensors.getCurrent(e_psens_ch1_battery) > 0.1f)
        wasCharging = true;
    
    if (wasCharging)
    {
        const float SoC = battery.getSoC();
        if (SoC < _chargeLimitSoC[1])
        {
            wasCharging = false;
            setCharger(true);
        }
        else if (SoC > _chargeLimitSoC[0])
        {
            setCharger(false);
        }
    }
}

void setup (void)
{
    pinMode(SWITCHER_PIN_CONSUMERS, OUTPUT);
    pinMode(SWITCHER_PIN_MOUNT, OUTPUT);
    pinMode(SWITCHER_PIN_CHARGER, OUTPUT);
    pinMode(SWITCHER_PIN_CAMERA, OUTPUT);

    analogWriteResolution(10);
    analogWriteFreq(150);
    pinMode(SWITCHER_PIN_HEATER1, OUTPUT);
    pinMode(SWITCHER_PIN_HEATER2, OUTPUT);
    #ifdef DEBUG_PRINT
    #warning Button LED output will not be used since this is the Serial Tx pin
    #else
    pinMode(SWITCHER_PIN_BTNLED, OUTPUT);
    #endif

    pinMode(SWITCHER_PIN_BUTTON, INPUT);
    attachInterrupt(digitalPinToInterrupt(SWITCHER_PIN_BUTTON), &_buttonISR, CHANGE);
}

buttonInfo getButtonInfo (void)
{
    buttonInfo bntCopy;
    noInterrupts();
    memcpy(&bntCopy, (const void *) &_btnInfo, sizeof(bntCopy));
    interrupts();
    return bntCopy;
}

void loop (void)
{
    _buttonLoop();
    switch (telegramBot.getAndClearCommand())
    {
        case e_noCommand: break;
        case e_powerConsumers_off:  _consumerPowerRequest = false;  break;
        case e_powerConsumers_on:   _consumerPowerRequest = true;   break;
    }

    _consumerPowerLoop(_consumerPowerRequest);
    _chargeSwitchLoop();
}

void setConsumers (const bool on)
{
    digitalWrite(SWITCHER_PIN_CONSUMERS, on);   // NO
}

void setMount (const bool on)
{
    digitalWrite(SWITCHER_PIN_MOUNT, !on);  // NC
}

void setCamera (const bool on)
{
    digitalWrite(SWITCHER_PIN_CAMERA, !on); // NC
}

void setCharger (const bool on)
{
    digitalWrite(SWITCHER_PIN_CHARGER, !on);
}

bool getCharger (void)
{
    return digitalRead(SWITCHER_PIN_CHARGER);
}

void setHeater (const uint_fast8_t heaterNr, const float powerLimit)
{
    const uint8_t pin = (heaterNr ? SWITCHER_PIN_HEATER2 : SWITCHER_PIN_HEATER1);
    int &prevPwmVal = _heaterPwmValues[heaterNr != 0];

    // TODO create a PID regulator
    
    float factor = powerLimit / 15.0f;          // 15 Watt equals maximum for now
    factor = fmaxf(0.0f, fminf(1.0f, factor));  // limit to 0..1

    int pwmValue = (int) (factor * 1023.0f);    // 10 bit resolution);

    // only update if the value differs
    if (pwmValue != prevPwmVal)
    {
        analogWrite(pin, pwmValue);
        prevPwmVal = pwmValue;
    }
}

bool getConsumers (void)
{
    return digitalRead(SWITCHER_PIN_CONSUMERS); // NO
}

bool getMount (void)
{
    return !digitalRead(SWITCHER_PIN_MOUNT);    // NC
}

bool getCamera (void)
{
    return !digitalRead(SWITCHER_PIN_CAMERA);   // NC
}

float getHeater (const uint_fast8_t heaterNr)
{
    const auto& pwmValue = _heaterPwmValues[heaterNr != 0];

    return ((float) pwmValue) / 10.23f;
}

int getButtonLedPwm (void)
{
    return _ledPwmValue;
}

keyLockState getKeyLockState (void)
{
    return _keyLockState;
}

consumersState getConsumersState (void)
{
    return _consumersState;
}

void setChargeLimit (const float startSoC, const float stopSoC)
{
    _chargeLimitSoC[0] = fminf(100.0f, fmaxf(0.0f, stopSoC));
    _chargeLimitSoC[1] = fminf(100.0f, fmaxf(0.0f, startSoC));
}

}