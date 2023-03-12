#include "energy_saver.hpp"
#include "communication/communication.hpp"

energySaver energy;

void energySaver::setup(void)
{
    // wifi_set_sleep_type(MODEM_SLEEP_T);
    WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 10);
    _modemSleep = true;
}

uint32_t energySaver::loop (const uint32_t maxSleepTime)
{
    // disable the LED 10 seconds after connect
    if (!WiFi.isConnected())
        _wifiConnSince = 0; // reset
    else if (!_wifiConnSince)
        _wifiConnSince = millis();
/*
    // automatic power save
    static bool powerSaveEnabled = false;
    if (comm.getMsSinceLastRx() > AUTO_DEEP_SLEEP_ACTIVATION_DELAY)
    {
        display.clearDisplay();
        display.display();
        stripe.turnOff();
        WiFi.disconnect(true);
        digitalWrite(LEDPIN, HIGH);
        delay(500);   // is this required so the modem actually gets turned off?
        ESP.deepSleep(0);
    }
    else if (comm.getMsSinceLastRx() > AUTO_POWER_SAVE_ACTIVATION_DELAY)
    {
        if (!powerSaveEnabled)
        {
            display.clearDisplay();
            display.setStatusBarVisible(true);
            display.loop();
            stripe.turnOff();
            _setModemSleep(true);
            powerSaveEnabled = true;
        }
    }
    else
        powerSaveEnabled = false;
*/
    // cylcic sleep
    uint32_t sleepDuration = 1;

    if (_modemSleep && maxSleepTime)
    {
        uint32_t sleepStart = millis();
        digitalWrite(LEDPIN, HIGH);
        delay(std::min<uint32_t>(maxSleepTime, _sleepLimit));

        if (!WiFi.isConnected() || ((millis() - _wifiConnSince) < 60000))
            digitalWrite(LEDPIN, LOW);
            
        sleepDuration = millis() - sleepStart;
    }
    else
        delay(10);   // let the system tasks run ("yield()" seems be not enough some times, i.e. when neither OLED nor the LED stripe get updated for several seconds)

    return sleepDuration;
}

void energySaver::_setModemSleep(const bool sleep)
{
    if (sleep != _modemSleep)
    {
        if (sleep)
            WiFi.setSleepMode(WIFI_LIGHT_SLEEP, 10);
        else
            WiFi.setSleepMode(WIFI_NONE_SLEEP);
    }

    _modemSleep = sleep;
}
