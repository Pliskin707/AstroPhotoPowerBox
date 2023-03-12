#ifndef __OLED_HPP__
#define __OLED_HPP__

#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "projutils/projutils.hpp"
#include "config.hpp"
#include "communication/communication_types.hpp"
#include "status_bar.hpp"
#include "energy saver/energy_saver.hpp"
#include "non volatile/non_volatile.hpp"

namespace pliskin
{
class statusDisplay: public Adafruit_SSD1306
{
    using Adafruit_SSD1306::Adafruit_SSD1306;   // "forward" all constructors

    private:
        statusBar _statusBar;
        uint32_t _nextBarUpdate = 0;
        const uint32_t _barUpdateDelay = 50;

    public:
        void setup (void);
        void loop (void);
        void showStartupScreen (void);
        int16_t centerText (const String &text);   // returns the start X-position of the text
        int16_t rightText (const String &text);    // returns the start X-position of the text
        void showWarning (const char * const text = nullptr);
        void printBarAt (const int y, const float percent, const char * const text = nullptr, const int height = 16);
        void setStatusBarVisible (const bool visible) {_nextBarUpdate = (visible ? 0 : UINT32_MAX);};
};

};

extern pliskin::statusDisplay display; // global OLED display instance

#endif