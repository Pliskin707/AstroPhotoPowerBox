#include "telegram_bot.hpp"

#define TBOT_COMMAND_ECHO   "/toggleecho"
#define TBOT_COMMAND_RELAYS "/relaycontrol"
#define TBOT_COMMAND_GETVER "/getboxversion"
#define TBOT_QUERY_RELAYS   "Relay"

#ifdef TELEGRAM_TOKEN
static WiFiClientSecure botClient;
static Session botSession;
static X509List ta(telegram_cert);
AstroTelegramBot telegramBot(botClient);
#endif

static void _pingRequested (const TBMessage &queryMsg)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf_P(PSTR("Ping: %s"), queryMsg.text);
    telegramBot.endQuery(queryMsg, "Pong", false);
}

bool AstroTelegramBot::begin()
{
    botClient.setSession(&botSession);
    botClient.setTrustAnchors(&ta);
    botClient.setBufferSizes(1024, 1024);

    setUpdateTime(3000);
    setTelegramToken(TELEGRAM_TOKEN);
    setFormattingStyle(AsyncTelegram2::FormatStyle::MARKDOWN);

    // keyboards
    _keyboard.addButton("Ping", "PingRequest", KeyboardButtonQuery, _pingRequested);
    addInlineKeyboard(&_keyboard);  // this is only required for keyboards with callback functions
    // sendTo(TELEGRAM_CHAT, "This is inline keyboard 1:", _keyboard.getJSON()); 

    // commands
    setMyCommands(TBOT_COMMAND_ECHO, "Schaltet Echo an oder aus");
    setMyCommands(TBOT_COMMAND_RELAYS, "Geräte ein- oder ausschalten");
    setMyCommands(TBOT_COMMAND_GETVER, "Liest die Softwareversion aus");
    
    if (!AsyncTelegram2::begin())
    {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.centerText("Telegram failed:");
        display.setCursor(0, 8);
        char buf[100];
        botClient.getLastSSLError(buf, sizeof(buf));
        display.setTextWrap(true);
        display.print(buf);
        display.setTextWrap(false);
        delay(1000);

        return false;
    }

    //sendTo(TELEGRAM_CHAT, "Hello World Async");
    _init_ok = true;
    return true;
}

void AstroTelegramBot::loop()
{
    uint_fast8_t maxReplies = 10;
    for (; maxReplies && getNewMessage(_msg); maxReplies--)
    {
        display.setCursor(0, 8);
        display.printf_P(PSTR("MsgType: %d"), _msg.messageType);
        display.setCursor(0, 16);
        display.setTextWrap(1);
        display.print(_msg.text);
        display.setTextWrap(false);

        switch (_msg.messageType)
        {
            case MessageQuery: _handleQuery(_msg); break;
            case MessageText: _handleText(_msg); break;
            default: break;
        }

        if (_echo)
        {
            _msg.disable_notification = true;
            sendMessage(_msg, "Echo: " + _msg.text);
        }
    }
}

void AstroTelegramBot::_handleQuery(const TBMessage &msg)
{
    if (msg.callbackQueryData.equals(TBOT_QUERY_RELAYS))
    {
        // TODO
        endQuery(msg, "Not yet implemented", true);
    }
    else if (msg.callbackQueryData.startsWith(F(TBOT_QUERY_RELAYS)))
    {
        char buf[50];
        int_fast8_t relayNr = (msg.callbackQueryData.charAt(strlen_P(TBOT_QUERY_RELAYS))) - '0';
        bool requestedState = msg.callbackQueryData.endsWith(F("On"));

        snprintf_P(buf, sizeof(buf), PSTR("Turning Relay[%d] %s"), relayNr, requestedState ? "ON":"OFF");
        buf[sizeof(buf) - 1] = 0;

        endQuery(msg, buf, false);
    }
    else
        endQuery(msg, String("Unknown command: " + msg.callbackQueryData).c_str(), true);
}

void AstroTelegramBot::_handleText(const TBMessage &msg)
{
    if (msg.text.equalsIgnoreCase(F(TBOT_COMMAND_RELAYS)))
    {
        InlineKeyboard kbd;
        bool relayStates[4] = {true, false, true, false};  // TODO fill these
        
        kbd.addButton(relayStates[0] ? "Verbraucher ausschalten":"Verbraucher anschalten", relayStates[0] ? "Relay0_Off":"Relay0_On", KeyboardButtonQuery);
        kbd.addRow();
        kbd.addButton(relayStates[1] ? "Ladegerät trennen":"Ladegerät verbinden", relayStates[1] ? "Relay1_Off":"Relay1_On", KeyboardButtonQuery);
        kbd.addRow();
        kbd.addButton(relayStates[2] ? "Montierung Notaus":"Montierung einschalten", relayStates[2] ? "Relay2_Off":"Relay2_On", KeyboardButtonQuery);
        kbd.addRow();
        kbd.addButton(relayStates[3] ? "Reserve an":"Reserve aus", relayStates[3] ? "Relay3_Off":"Relay3_On", KeyboardButtonQuery);

        sendMessage(msg, "*__Relaissteuerung__*", kbd);
    }
    else if (msg.text.equalsIgnoreCase(F(TBOT_COMMAND_ECHO)))
    {
        _echo ^= true;
        char buf[30];
        snprintf_P(buf, sizeof(buf), PSTR("Echo is now %s"), _echo ? "on":"off");
        buf[sizeof(buf) - 1] = 0;   // force end of string
        sendMessage(msg, buf);
    }
    else if (msg.text.equalsIgnoreCase(F(TBOT_COMMAND_GETVER)))
    {
        char buf[150];
        snprintf_P(buf, sizeof(buf), PSTR("*__%s__*\n\n`Version: %u\\.%u\\.%u\nAuthor:  Pliskin707`\n[Github Repository](https://github.com/Pliskin707/AstroPhotoPowerBox)"), SERVICE_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        buf[sizeof(buf) - 1] = 0;   // force end of string
        sendMessage(msg, buf);
    }
}
