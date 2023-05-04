#include "ota.hpp"
#include "config.hpp"
#include <signal.h>

namespace pliskin
{

static statusDisplay * _pDisplay = nullptr;
static volatile sig_atomic_t _sigUpdateStarted = false;

void ota::begin (const char * const deviceName, statusDisplay * const pDisplay)
{
    ArduinoOTA.setHostname(deviceName);
    _pDisplay = pDisplay;

    ArduinoOTA.onStart([]() 
    {
        if (ready4Update())
        {
            _sigUpdateStarted = true;
            wifi_set_sleep_type(NONE_SLEEP_T);
            LittleFS.end();

            if (_pDisplay)
            {
                _pDisplay->clearDisplay();
                _pDisplay->setCursor(0, 0);
                _pDisplay->centerText("Updating");
                _pDisplay->display();
            }
        }
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
    {
        static unsigned int callCount = 0;

        if (_pDisplay)
        {
            
            if (((callCount % 10) == 0) || (progress == total))
            {
                float progressPercent = 100.0f * ((float) progress) / ((float) total);
                char str[40];
                snprintf_P(str, sizeof(str), PSTR("%d/%d (%u%%)"), progress, total, (uint8_t) progressPercent);
                _pDisplay->printBarAt(16, progressPercent, str);
                _pDisplay->display();
            }
        }

        callCount++;
    });

    ArduinoOTA.onError([](ota_error_t error) 
    {
        _sigUpdateStarted = false;
        if (_pDisplay)
        {
            _pDisplay->clearDisplay();
            _pDisplay->setCursor(0, 16);
            _pDisplay->printf_P(PSTR("Update failed (%d)"), error);
            _pDisplay->suppressContentTemporary(3000);
        }
    });

    ArduinoOTA.onEnd([]() 
    {
        switcher::setConsumers(false);
        switcher::setHeater(0, 0);
        switcher::setHeater(1, 0);
        if (_pDisplay)
        {
            _pDisplay->clearDisplay();
            _pDisplay->setCursor(0, 0);
            _pDisplay->centerText("Done");
            _pDisplay->print(F("\n\nreboot..."));
            _pDisplay->display();
        }
        ESP.reset();
    });

    ArduinoOTA.begin(false);
}

void ota::handle(void)
{
    if (ready4Update())
        ArduinoOTA.handle();
}

bool ota::isUpdating (void)
{
    return _sigUpdateStarted != 0;
}

bool ota::ready4Update(void)
{
    return switcher::getConsumersState() == switcher::consumersState::e_consumers_off_idle;
}

};