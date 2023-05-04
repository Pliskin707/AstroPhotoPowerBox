#include "telegram_bot.hpp"

#define TBOT_COMMAND_ECHO       "/toggleecho"
#define TBOT_COMMAND_RELAYS     "/relaycontrol"
#define TBOT_COMMAND_GETVER     "/getboxversion"
#define TBOT_COMMAND_HEATER     "/setheater"
#define TBOT_COMMAND_STATUS     "/getstatus"
#define TBOT_QUERY_RELAYS       "Relay"

#define TBOT_HEATER_OFF         "\xE2\x9D\x84 Aus"
#define TBOT_HEATER_5W          "\xF0\x9F\x92\xA7 5W"
#define TBOT_HEATER_10W         "\xE2\x99\xA8 10W"
#define TBOT_HEATER_15W         "\xF0\x9F\x94\xA5 15W"
#define TBOT_HEATER_AUTO        "\xF0\x9F\x93\x89 Auto"
// #define TBOT_CANCEL             "\xE2\x9C\x96 Abbrechen"
#define TBOT_CANCEL             "Abbrechen"

#ifdef TELEGRAM_TOKEN
static WiFiClientSecure botClient;
static Session botSession;
static X509List ta(telegram_cert);
AstroTelegramBot telegramBot(botClient);
#endif

static  pliskin_ringbuffer::rb_base _pendingCommands = pliskin_ringbuffer::rb_base(sizeof(e_telegram_command), 10);

static void _abortKeyboard (const TBMessage &queryMsg)
{
    String editText = "Befehl \\\"" + queryMsg.text;
    editText += "\\\" durch ";
    editText += queryMsg.sender.firstName;
    editText += " abgebrochen";
    telegramBot.editMessage(queryMsg, editText, "");
    telegramBot.endQuery(queryMsg, "abgebrochen", false);
}

static void _pingRequested (const TBMessage &queryMsg)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf_P(PSTR("Ping: %s"), queryMsg.text);
    display.suppressContentTemporary(1000);
    telegramBot.endQuery(queryMsg, "Pong", false);
}

static void _subrelayConsumers (const TBMessage &queryMsg)
{
    bool requestedState = !switcher::getConsumers();    // TODO make selectable with another keyboard

    // [inline mention of a user](tg://user?id=123456789)
    String editText = "[" + queryMsg.sender.firstName;
    editText += " ";
    editText += queryMsg.sender.lastName;
    editText += "]\nVerbraucher werden ";
    editText += (requestedState ? "eingeschaltet":"ausgeschaltet");
    telegramBot.editMessage(queryMsg, editText, "");
    telegramBot.endQuery(queryMsg, "", false);

    const e_telegram_command cmd = (requestedState ? e_powerConsumers_on : e_powerConsumers_off);
    _pendingCommands.write(&cmd);
}

static void _subrelayNotImplemented (const TBMessage &queryMsg)
{
    // [inline mention of a user](tg://user?id=123456789)
    String editText = "[" + queryMsg.sender.firstName;
    editText += " ";
    editText += queryMsg.sender.lastName;
    editText += "](tg://user?id=";
    editText += String(queryMsg.sender.id);
    editText += "\\) ";
    editText += " User:";
    editText += queryMsg.sender.username;
    telegramBot.editMessage(queryMsg, editText, "");
    telegramBot.endQuery(queryMsg, "Not yet implemented", true);
}

bool AstroTelegramBot::begin()
{
    if (!_keyboardsInitialized)
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

        _relayKeyboard.addButton("\xE2\x9A\xA1 Verbraucher", "RelayConsumers", KeyboardButtonQuery, _subrelayConsumers);
        _relayKeyboard.addRow();
        _relayKeyboard.addButton("\xF0\x9F\x94\x8C Ladegerät", "RelayCharger", KeyboardButtonQuery, _subrelayNotImplemented);
        _relayKeyboard.addRow();
        _relayKeyboard.addButton("\xF0\x9F\x94\xAD Montierung", "RelayMount", KeyboardButtonQuery, _subrelayNotImplemented);
        _relayKeyboard.addRow();
        _relayKeyboard.addButton(TBOT_CANCEL, "AbortKeyboard", KeyboardButtonQuery, _abortKeyboard);
        // 	\xF0\x9F\x92\xBB computer
        // \xF0\x9F\x94\x8B battery
        // \xF0\x9F\x94\xAD telescope
        // \xF0\x9F\x93\xA6 package
        // \xE2\x9A\xA0 warning
        addInlineKeyboard(&_relayKeyboard);

        _heaterReplyKeyboard.enableOneTime();
        _heaterReplyKeyboard.addButton(TBOT_HEATER_OFF);
        _heaterReplyKeyboard.addButton(TBOT_HEATER_5W);
        _heaterReplyKeyboard.addButton(TBOT_HEATER_10W);
        _heaterReplyKeyboard.addButton(TBOT_HEATER_15W);
        _heaterReplyKeyboard.addButton(TBOT_HEATER_AUTO);
        _heaterReplyKeyboard.addRow();
        _heaterReplyKeyboard.addButton(TBOT_CANCEL);

        // commands
        setMyCommands(TBOT_COMMAND_ECHO, "Schaltet Echo an oder aus");
        setMyCommands(TBOT_COMMAND_RELAYS, "Geräte ein- oder ausschalten");
        setMyCommands(TBOT_COMMAND_GETVER, "Liest die Softwareversion aus");
        setMyCommands(TBOT_COMMAND_HEATER, "Steuert ein Heizband");
        setMyCommands(TBOT_COMMAND_STATUS, "Gibt den aktuellen Status zurück");

        _keyboardsInitialized = true;
    }
    
    if (!AsyncTelegram2::begin())
    {
        // display.clearDisplay();
        // display.setCursor(0, 0);
        // display.centerText("Telegram failed:");
        // display.setCursor(0, 8);
        // char buf[100];
        // botClient.getLastSSLError(buf, sizeof(buf));
        // display.setTextWrap(true);
        // display.print(buf);
        // display.setTextWrap(false);
        // display.suppressContentTemporary(10000);

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
        display.suppressContentTemporary(1000);

        switch (_msg.messageType)
        {
            case MessageQuery: _handleQuery(_msg); break;
            case MessageText: _handleText(_msg); break;     // messages sent to the bot (no keyboard button presses)
            case MessageReply: _handleReplies(_msg); break; // reply keyboard replies
            default: break;
        }

        if (_echo)
        {
            _msg.disable_notification = true;
            sendMessage(_msg, "Echo: " + _msg.text);
        }
    }
}

e_telegram_command AstroTelegramBot::getAndClearCommand(void) 
{
    e_telegram_command cmd = e_noCommand;
    _pendingCommands.read(&cmd);
    
    return cmd;
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
        switch (relayNr)
        {
            case 0: switcher::setConsumers(requestedState); break;
            case 1: switcher::setCharger(requestedState); break;
            case 2: switcher::setMount(requestedState); break;
        }
    }
    else
        endQuery(msg, String("Unknown command: " + msg.callbackQueryData).c_str(), true);
}

void AstroTelegramBot::_handleText(const TBMessage &msg)
{
    if (msg.text.startsWith(F(TBOT_COMMAND_RELAYS)))
    {
        sendMessage(msg, "*__Relaissteuerung__*", _relayKeyboard);
    }
    else if (msg.text.startsWith(F(TBOT_COMMAND_ECHO)))
    {
        _echo ^= true;
        char buf[30];
        snprintf_P(buf, sizeof(buf), PSTR("Echo is now %s"), _echo ? "on":"off");
        buf[sizeof(buf) - 1] = 0;   // force end of string
        sendMessage(msg, buf);
    }
    else if (msg.text.startsWith(F(TBOT_COMMAND_HEATER)))
    {
        sendMessage(msg, "Heizbandsteuerung", _heaterReplyKeyboard);
        _heaterKeyboardVisible = true;
    }
    else if (msg.text.startsWith(F(TBOT_COMMAND_GETVER)))
    {
        char buf[150];
        snprintf_P(buf, sizeof(buf), PSTR("*__%s__*\n\n`Version: %u\\.%u\\.%u\nAuthor:  Pliskin707`\n[Github Repository](https://github.com/Pliskin707/AstroPhotoPowerBox)"), SERVICE_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
        buf[sizeof(buf) - 1] = 0;   // force end of string
        sendMessage(msg, buf);
    }
    else if (msg.text.startsWith(F(TBOT_COMMAND_STATUS)))
    {
        char buf[150];
        div_t volt = div(powersensors.getVoltage(e_psens_ch1_battery) * 100, 100);
        div_t amp = div(powersensors.getCurrent(e_psens_ch1_battery) * 1000, 1000);

        snprintf_P(buf, sizeof(buf), PSTR("*__Status__*\n\nState of Charge: %d %%\nVerbleibende Ladung: %d Wh\nSpannung: %s%d,%02u V\nStrom: %s%d,%03u A"), 
            (uint16_t) battery.getSoC(), (uint16_t) battery.getEnergyRemainingWh(), 
            volt.rem < 0 ? "\\-":"", volt.quot, abs(volt.rem), 
            amp.rem < 0 ? "\\-":"", amp.quot, abs(amp.rem));
        buf[sizeof(buf) - 1] = 0;
        sendMessage(msg, buf);
    }
    else if (msg.text.startsWith(F(TBOT_CANCEL)))
    {
        // ReplyKeyboard emptyKeyboard;
        // sendMessage(msg, "Aktion abgebrochen", emptyKeyboard);
        removeReplyKeyboard(msg, "Aktion abgebrochen");
    }
}

void AstroTelegramBot::_handleReplies(const TBMessage &msg)
{
    if (msg.text.startsWith(F(TBOT_CANCEL)))
    {
        removeReplyKeyboard(msg, "Aktion abgebrochen");
    }
    else
    {
        char buf[100];
        snprintf_P(buf, sizeof(buf), PSTR("%s gewählt von %s"), msg.text.c_str(), msg.sender.firstName.c_str());
        buf[sizeof(buf) - 1] = 0;

        // TODO make this work for both heaters
        e_telegram_command cmd = e_noCommand;
        if (msg.text.equals(TBOT_HEATER_OFF))       cmd = e_htr1_off;
        else if (msg.text.equals(TBOT_HEATER_5W))   cmd = e_htr1_5W;
        else if (msg.text.equals(TBOT_HEATER_10W))  cmd = e_htr1_10W;
        else if (msg.text.equals(TBOT_HEATER_15W))  cmd = e_htr1_15W;
        else if (msg.text.equals(TBOT_HEATER_AUTO)) cmd = e_htr1_auto;

        if (cmd != e_noCommand)
            _pendingCommands.write(&cmd);

        removeReplyKeyboard(msg, buf);
    }
}
