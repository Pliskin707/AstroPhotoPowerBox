#ifndef __TELEGRAM_BOT_H__
#define __TELEGRAM_BOT_H__

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include "AsyncTelegram2.h"

#include "config.h"
#include "wifiauth2.h"
#include "oled/oled.hpp"

class AstroTelegramBot : public AsyncTelegram2
{
    using AsyncTelegram2::AsyncTelegram2;

    private:
        bool _init_ok = false;
        bool _echo = false;
        bool _heaterKeyboardVisible = false;
        uint32_t _nextCheck = 0;
        TBMessage _msg;
        InlineKeyboard _keyboard;
        InlineKeyboard _relayKeyboard;
        ReplyKeyboard _heaterReplyKeyboard;

        void _handleQuery (const TBMessage &msg);
        void _handleText (const TBMessage &msg);
        void _handleReplies (const TBMessage &msg);

    public:
        bool begin ();
        void loop ();
};

extern AstroTelegramBot telegramBot;

#endif // __TELEGRAM_BOT_H__