#ifndef __SCREEN_CONTENT_H__
#define __SCREEN_CONTENT_H__

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

#include "screen_types.hpp"
#include "battery/battery.hpp"
#include "sensors/power/psens.hpp"
#include "communication/communication.hpp"

namespace pliskin
{

class startupScreen : public screenBaseClass
{
    public:
        uint32_t show (Adafruit_SSD1306 * const pOled) override;
        e_screen getType (void) const override {return pliskin::e_screen::startup;};
};

class homeScreen : public screenBaseClass
{
    public:
        uint32_t show (Adafruit_SSD1306 * const pOled) override;
        e_screen getType (void) const override {return pliskin::e_screen::home;};
};

class powerScreen : public screenBaseClass
{
    private:
        e_psens_channel _sensors[2];

    public:
        uint32_t show (Adafruit_SSD1306 * const pOled) override;
        e_screen getType (void) const override {return pliskin::e_screen::home;};

        explicit powerScreen (const e_psens_channel top, const e_psens_channel bottom);
};

class idleScreen : public screenBaseClass
{
    public:
        uint32_t show (Adafruit_SSD1306 * const pOled) override {pOled->clearDisplay(); return UINT32_MAX;};
        e_screen getType (void) const override {return pliskin::e_screen::home;};
};

class offScreen : public idleScreen {
    public:
        e_screen getType (void) const override {return pliskin::e_screen::off;};
};

}

#endif // __SCREEN_CONTENT_H__