#ifndef __TELEGRAM_BOT_H__
#define __TELEGRAM_BOT_H__

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include "UniversalTelegramBot.h"

#include "config.h"
#include "wifiauth2.h"
#include "oled/oled.hpp"

class AstroTelegramBot : public UniversalTelegramBot
{
    using UniversalTelegramBot::UniversalTelegramBot;

    private:
    bool _init_ok = false;
    uint32_t _nextCheck = 0;

    public:
    bool begin ();
    void loop ();
};

extern AstroTelegramBot telegramBot;

#endif // __TELEGRAM_BOT_H__