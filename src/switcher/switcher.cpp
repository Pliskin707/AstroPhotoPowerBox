#include "switcher.hpp"

namespace switcher 
{

static volatile buttonInfo _btnInfo = {{0}};
static float _chargeLimitSoC[2] = {100.0f, 95.0f};  // stop and restart SoC

static void IRAM_ATTR _buttonISR (void)
{
    const bool state        = digitalRead(SWITCHER_PIN_BUTTON);
    const uint32_t sysTime  = millis();

    if (state)
    {
        _btnInfo.lastButtonPressTime = sysTime;
        _btnInfo.numPressesSinceStart++;
    }
    else
        _btnInfo.lastButtonReleaseTime = sysTime;

    _btnInfo.pressed = state;
}

static void _setLedPwm (const uint16_t pwmVal)
{
    #ifdef DEBUG_PRINT
    (void) pwmVal;  // don't do anything since this is the Serial Tx pin
    #else
    analogWrite(SWITCHER_PIN_BTNLED, pwmVal);
    #endif
}

static bool _buttonLoop (void)
{
    static bool consumerPowerRequest = false;
    static enum {
        e_locked_wait4release,
        e_locked_idle, 
        e_unlocking_hold, 
        e_unlocking_wait4release, 
        e_unlocking_release_delay,
        e_unlocking_wait4ack,
        e_unlocked_idle} keyLockState = e_locked_idle;

    const uint32_t sysTime = millis();
    const buttonInfo btnInfo = getButtonInfo();

    switch (keyLockState)
    {
        case e_locked_wait4release:
        {
            if (!btnInfo.pressed)
                keyLockState = e_locked_idle;
        }
        break;

        case e_locked_idle:
        {
            _setLedPwm(5);

            // wait for a button press
            if (btnInfo.pressed)
            {
                keyLockState = e_unlocking_hold;
                dprintf("Button pressed. Starting unlock sequence...\n")
            }
        }
        break;

        case e_unlocking_hold:
        {
            _setLedPwm(0x8000);

            // keep the button presse for about one second
            if (!btnInfo.pressed)
            {
                keyLockState = e_locked_idle;
                dprintf("Button released too early. Returning to idle state.\n")
            }
            else if ((sysTime - btnInfo.lastButtonPressTime) >= 1000)
            {
                keyLockState = e_unlocking_wait4release;
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
                keyLockState = e_unlocking_release_delay;
                dprintf("Button released. Waiting for completion delay...\n")
            }
            else if ((sysTime - btnInfo.lastButtonPressTime) >= 2000)
            {
                keyLockState = e_locked_wait4release;
                dprintf("Button was not released. Returning to initialization.\n")
            }
        }
        break;

        case e_unlocking_release_delay:
        {
            // button must not be pressed during this time
            if (btnInfo.pressed)
            {
                keyLockState = e_locked_wait4release;
                dprintf("Button pressed too early. Returning to initialization.\n")
            }
            else if ((sysTime - btnInfo.lastButtonReleaseTime) >= 1000)
            {
                keyLockState = e_unlocking_wait4ack;
                dprintf("Completion delay completed. Waiting for activation...\n")
            }
        }
        break;

        case e_unlocking_wait4ack:
        {
            _setLedPwm(0xffff);

            // wait for a final button press to acknowledge the unlock procedure
            if (btnInfo.pressed)
            {
                keyLockState = e_unlocked_idle;
                dprintf("Activation complete. Button will remain unlocked for some time.\n")
            }
            else if ((sysTime - btnInfo.lastButtonReleaseTime) >= 1000)
            {
                keyLockState = e_locked_idle;
                dprintf("Activation window expired. Returning to idle state.\n")
            }
        }
        break;

        case e_unlocked_idle:
        {
            const int ledPwmVal = (millis() & (1 << 10)) ? 0 : 0x1000; // toggle about every second
            _setLedPwm(ledPwmVal);

            // auto-lock after a minute
            if (((sysTime - btnInfo.lastButtonReleaseTime) >= 60000) && 
                ((sysTime - btnInfo.lastButtonPressTime) >= 60000))
            {
                // no activity within the last minute
                keyLockState = e_locked_wait4release;
                dprintf("Button locked due to inactivity.\n")
            }
        }
        break;
    }

    static uint32_t prevBtnPressCount = 0;
    if ((btnInfo.numPressesSinceStart != prevBtnPressCount) && (keyLockState == e_unlocked_idle))
    {
        // toggle the consumers power request
        consumerPowerRequest ^= true;
        dprintf("User power request is now %s\n", (consumerPowerRequest ? "ON":"OFF"))
    }

    prevBtnPressCount = btnInfo.numPressesSinceStart;
    return consumerPowerRequest;
}

static void _consumerPowerLoop (const bool requestFromButton)
{
    static enum 
    {
        e_consumers_off_idle,
        e_consumers_off_prepare_activation,
        e_consumers_on_subs_off,
        e_consumers_on_subs_activation,
        e_consumers_on_idle,
        e_consumers_on_prepare_power_down,
        e_consumers_on_wait4pc_shutdown
    } consumersState = e_consumers_off_idle;
    const auto prevState = consumersState;
    static uint32_t lastStateChange = 0;
    const uint32_t timeSinceStateChange = millis() - lastStateChange;

    switch (consumersState)
    {
        case e_consumers_off_idle:
        {
            if (requestFromButton)
                consumersState = e_consumers_off_prepare_activation;
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

            // check state
            if ((getHeater(0) < 10) && (getHeater(1) < 10) && !getMount() && !getCamera())
                consumersState = e_consumers_on_subs_off;
        }
        break;

        case e_consumers_on_subs_off:
        {
            setConsumers(true);
            const float voltage = powersensors.getVoltage(e_psens_ch5_mount);
            if ((timeSinceStateChange > 100) && (voltage >= BAT_MIN_TURN_ON_VOLTAGE)) // wait for capacities to load
                consumersState = e_consumers_on_subs_activation;
            else if (timeSinceStateChange > 3000)
                consumersState = e_consumers_off_idle;  // TODO generate a warning message (telegram?)
        }
        break;

        case e_consumers_on_subs_activation:
        {
            setCamera(true);

            if (timeSinceStateChange >= 500) 
                setMount(true);

            if (timeSinceStateChange >= 1000) 
                consumersState = e_consumers_on_idle;
        }
        break;

        case e_consumers_on_idle:
        {
            if (!requestFromButton)
                consumersState = e_consumers_on_prepare_power_down;
        }
        break;

        case e_consumers_on_prepare_power_down:
        {
            setMount(false);
            // leave the camera on in case some data transmission is going on
            consumersState = e_consumers_on_wait4pc_shutdown;
        }
        break;

        case e_consumers_on_wait4pc_shutdown:
        {
            if (powersensors.getCurrent(e_psens_ch4_pc) < 0.4)  // pc may use up to 300 mA while powered down
                consumersState = e_consumers_off_idle;
        }
        break;
    }

    if (consumersState != prevState)
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

    analogWriteResolution(16);
    pinMode(SWITCHER_PIN_HEATER1, OUTPUT);
    pinMode(SWITCHER_PIN_HEATER2, OUTPUT);
    #ifdef DEBUG_PRINT
    #warning Button LED output will not be used since this is the Serial Tx pin
    #else
    pinMode(SWITCHER_PIN_BTNLED, OUTPUT);
    #endif

    pinMode(SWITCHER_PIN_BUTTON, INPUT);
    attachInterrupt(digitalPinToInterrupt(SWITCHER_PIN_BUTTON), &_buttonISR, FALLING);
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
    const bool requestedConsumerState = _buttonLoop();
    _consumerPowerLoop(requestedConsumerState);
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

    // TODO create a PID regulator
    
    float factor = powerLimit / 15.0f;          // 15 Watt equals maximum for now
    factor = fmaxf(1.0f, fminf(0.0f, factor));  // limit to 0..1

    int pwmValue = (int) (factor * 65535.0f);   // 16 bit resolution);
    analogWrite(pin, pwmValue);
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
    const uint8_t pin = (heaterNr ? SWITCHER_PIN_HEATER2 : SWITCHER_PIN_HEATER1);
    const auto& pwmValue = analogRead(pin);

    return ((float) pwmValue) / 655.35f;
}

void setChargeLimit (const float startSoC, const float stopSoC)
{
    _chargeLimitSoC[0] = fminf(100.0f, fmaxf(0.0f, stopSoC));
    _chargeLimitSoC[1] = fminf(100.0f, fmaxf(0.0f, startSoC));
}

}