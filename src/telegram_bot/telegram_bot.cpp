#include "telegram_bot.hpp"

#ifdef TELEGRAM_TOKEN
static WiFiClientSecure botClient;
static X509List ta((const uint8_t *) TELEGRAM_CERTIFICATE_ROOT, sizeof(TELEGRAM_CERTIFICATE_ROOT));
AstroTelegramBot telegramBot(TELEGRAM_TOKEN, botClient);
#endif

bool AstroTelegramBot::begin()
{
    botClient.setTrustAnchors(&ta);
    
    if (!telegramBot.sendChatAction(TELEGRAM_CHAT, "typing"))
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

    _init_ok = true;
    return true;
}

void AstroTelegramBot::loop()
{
    const uint32_t sysTime = millis();

    if (sysTime >= _nextCheck)
    {
        _nextCheck = sysTime + 3000;
        uint32_t timeout = sysTime + 100;

        do
        {
            const auto unreadMsgs = getUpdates(last_message_received + 1);
            if (!unreadMsgs)
                break;

            for (int ii = 0; ii < unreadMsgs; ii++)
            {
                const auto &msg = messages[ii];
                // echo
                sendMessage(msg.chat_id, msg.text);
            }
            
        } while (millis() < timeout);
    }
}
