#ifndef __TELEGRAM_BOT_H__
#define __TELEGRAM_BOT_H__

#include <Arduino.h>
#include <queue.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>
#include "AsyncTelegram2.h"

#include "config.h"
#include "rb.h"
#include "wifiauth2.h"
#include "oled/oled.hpp"
#include "switcher/switcher.hpp"

typedef enum
{
    e_noCommand = 0,
    e_powerConsumers_on,
    e_powerConsumers_off,
    e_htr1_off,
    e_htr1_5W,
    e_htr1_10W,
    e_htr1_15W,
    e_htr1_auto,
    e_htr2_off,
    e_htr2_5W,
    e_htr2_10W,
    e_htr2_15W,
    e_htr2_auto
} e_telegram_command;

class AstroTelegramBot : public AsyncTelegram2
{
    using AsyncTelegram2::AsyncTelegram2;

    private:
        bool _init_ok = false;
        bool _keyboardsInitialized = false;
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
        e_telegram_command getAndClearCommand (void);
};

extern AstroTelegramBot telegramBot;

#endif // __TELEGRAM_BOT_H__