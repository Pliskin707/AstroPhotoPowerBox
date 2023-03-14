#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClientSecure.h>

#include "ota/ota.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"
#include "oled/oled.hpp"
#include "mavg.hpp"
#include "communication/communication.hpp"
#include "energy saver/energy_saver.hpp"
#include "non volatile/non_volatile.hpp"
#include "telegram_bot/telegram_bot.hpp"

using namespace pliskin;

/** Wifi authentication **
 * 
 * this file needs to be created with the following content (and is obviously not included in version control):
 * 
 * #pragma once
 * 
 * #define SSID "<YourSSIDhere>"
 * #define PASSWORD "<YourPasswordHere>"
 * #define TELEGRAM_TOKEN "<BotTokenHere>"  // optional
 * #define TELEGRAM_CHAT "<ChatIDHere>"     // optional
 */
#include "wifiauth2.h"

static bool mDNS_init_ok = false;


void setup() {
  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  #else
  Serial.begin(115200);
  #endif

  display.setup();
  configTime(TIMEZONESTR, NTPSERVERSTR);

  // Wifi
  WiFi.hostname(DeviceName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);

  wl_status_t wstat;
  while (true)
  {
    delay(500);
    wstat = WiFi.status();
    if (wstat == WL_CONNECTED)
      break;

    #ifndef DEBUG_PRINT
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    #endif
    dprintf("Connecting (%d) ...\n", wstat);
  };

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  telegramBot.begin();

  // mDNS
  mDNS_init_ok = MDNS.begin(DeviceName);
  if (mDNS_init_ok)
    comm.setup();

  // OTA
  ota::begin(DeviceName.c_str(), &display);

  energy.setup();
  energy.setSleepLimit(MAX_SLEEP_DURATION_MS);

  display.clearDisplay();
}

void loop() {
  if (WiFi.isConnected())
  {
    if (mDNS_init_ok)
      MDNS.update();

    ota::handle();

    if (ota::isUpdating())
    {
      // don't do anything else (ota handles the oled display)
      yield();
    }
    else
    {
      comm.loop();
      telegramBot.loop();
      if (comm.hasNewData())
      {       
        // TODO
      }

      display.loop();
      comm.transmit();  // call this last so the status is as up-to-date as possible
      nvmem.loop();
      energy.loop();
    }
  }
  else
  {
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));

    display.clearDisplay();
    display.setCursor(0, 0);
    display.showWarning("WiFi Lost");
    display.display();

    // WiFi.begin();
    delay(1000);
  }
}