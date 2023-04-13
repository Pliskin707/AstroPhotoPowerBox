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
#include "sensors/power/psens.hpp"
#include "battery/battery.hpp"
#include "switcher/switcher.hpp"

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
  switcher::setup();

  #ifdef DEBUG_PRINT
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  #else
  Serial.end();
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

  // TODO read last known values from eeprom/file system
  // TODO sensor calibration

  powersensors.setup();

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

      comm.transmit();  // call this last so the status is as up-to-date as possible
    }
  }
  else
  {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.showWarning("WiFi Lost");
    display.display();
  }

  switcher::loop();
  nvmem.loop();
  energy.loop(0);
  powersensors.loop();
  //battery.loop();
  display.loop();

  // #ifdef DEBUG_PRINT
  // static uint32_t nextDebugPower = 0;
  // if (millis() > nextDebugPower)
  // {
  //   nextDebugPower = millis() + 1000;
  //   Serial.flush();

  //   float u = 1.0f, i = 0.01f, p = 100.0f;
  //   for (uint8_t channel = 0; channel < e_psens_num_channels; channel++)
  //   {
  //     const e_psens_channel e_ch = static_cast<e_psens_channel>(channel);
  //     // u = powersensors.getVoltage(e_ch);
  //     i = powersensors.getCurrent(e_ch);
  //     // p = powersensors.getPower(e_ch);
  //     div_t 
  //       qru = div(u * 100, 100),
  //       qri = div(i * 10000, 10000),
  //       qrp = div(p * 100, 100);

  //     Serial.printf_P(PSTR("CH%u;U = %2d.%05u;I = %2d.%02u;P = %2d.%02u\n"), channel, qru.quot, qru.rem, qri.quot, qri.rem, qrp.quot, qrp.rem);
  //     delay(200);
  //   }
  //   Serial.print('\n');
  // }
  // #endif
}