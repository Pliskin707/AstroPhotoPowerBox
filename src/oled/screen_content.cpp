#include "screen_content.hpp"

namespace pliskin {
    uint32_t startupScreen::show(Adafruit_SSD1306 * const pOled)
    {
        pOled->clearDisplay();
        pOled->setCursor(0, 0);
        pOled->setTextSize(2);
        pOled->print(DEVICE_PREFIX);
        pOled->setTextSize(1);
        pOled->setCursor(0, 16);
        pOled->print(F("Version: "));
        pOled->print(VERSION_STRING);
        pOled->setCursor(0, 24);
        pOled->print(WiFi.macAddress().c_str());
        pOled->display();

        return UINT32_MAX;
    }

    uint32_t homeScreen::show(Adafruit_SSD1306 * const pOled)
    {
        const int16_t fillHeight = pOled->height() - 2 * 2;
        const int16_t fillWidth = fillHeight * 2;
        const int16_t rectWidth = fillWidth + 2 * 2;
        const float SoC         = battery.getSoC();
        int16_t iSoC = (int16_t) ((fmaxf(0.0f, fminf(100.0f, SoC)) / 100.0f) * fillWidth);

        // carving method
        pOled->fillRect(0, 0, rectWidth, pOled->height(), SSD1306_WHITE);
        pOled->fillRect(rectWidth, 8, 3, (pOled->height() - 2 * 8), SSD1306_WHITE);
        pOled->drawRect(1, 1, rectWidth - 2, pOled->height() - 2, SSD1306_BLACK);

        // invert the content of the empty part
        for (int fillLine = 0; fillLine < fillHeight; fillLine++)
        {
            for (int vPos = iSoC + (fillLine / 3); vPos < fillWidth; vPos++)  // slightly angeled
            {
                const int16_t x = vPos + 2, y = fillLine + 2;
                pOled->drawPixel(x, y, !pOled->getPixel(x, y));
            }
        }

        // print text info
        pOled->setTextSize(2);
        pOled->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        pOled->setCursor(rectWidth + 8, 0);
        pOled->printf_P(PSTR("%3d"), (int) SoC);
        pOled->setTextSize(1);
        pOled->setCursor(pOled->getCursorX(), 8);
        pOled->print('%');
        // printf_P(PSTR("%03.0f%%"), SoC);
        pOled->setTextSize(2);
        pOled->setCursor(rectWidth + 8, 16);
        pOled->printf_P(PSTR("%3d"), (int) battery.getEnergyRemaining());
        pOled->setTextSize(1);
        pOled->setCursor(pOled->getCursorX(), 24);
        pOled->print("Wh");
        // printf_P(PSTR("%3d Wh"), battery.getEnergyRemaining());

        // restore default values
        pOled->setTextSize(1);
        pOled->setTextColor(SSD1306_WHITE, SSD1306_BLACK);

        return millis() + 1000;
    }

    static size_t printChannelName (Adafruit_SSD1306 * const pOled, const e_psens_channel channel)
    {
        size_t len = 0;

        switch (channel)
        {
            case e_psens_ch1_battery:       len = pOled->print(F("Battery")); break;
            case e_psens_ch2_dew_heater_1:  len = pOled->print(F("Heater1")); break;
            case e_psens_ch3_dew_heater_2:  len = pOled->print(F("Heater2")); break;
            case e_psens_ch4_pc:            len = pOled->print(F("Computer")); break;
            case e_psens_ch5_mount:         len = pOled->print(F("Mount")); break;
            case e_psens_ch6_imaging_cam:   len = pOled->print(F("Camera")); break;
            case e_psens_num_channels:      len = pOled->print(F("<?>")); break;
        }

        return len;
    }

    powerScreen::powerScreen (const e_psens_channel top, const e_psens_channel bottom)
    {
        _sensors[0] = (top < e_psens_num_channels) ? top : e_psens_num_channels;
        _sensors[1] = (bottom < e_psens_num_channels) ? bottom : e_psens_num_channels;
    }

    uint32_t powerScreen::show (Adafruit_SSD1306 * const pOled)
    {
        int16_t y = 0;
        for (const auto channel : _sensors)
        {
            pOled->setCursor(0, y);
            pOled->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            pOled->print(' ');
            printChannelName(pOled, channel);
            pOled->print(':');
            pOled->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
            pOled->setCursor(60, y);
            pOled->printf_P(PSTR("%+02.3fW\n"), powersensors.getPower(channel));
            pOled->printf_P(PSTR("%+02.1fV %+01.3fA (%d)"),
                powersensors.getVoltage(channel),
                powersensors.getCurrent(channel),
                powersensors.getAdcShunt(channel));

            y += 16;
        }

        return millis() + 200;
    }


}