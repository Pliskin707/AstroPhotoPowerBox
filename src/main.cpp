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
#include "connection/connection.hpp"

using namespace pliskin;

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
  connectionHandler::setup();

  // OTA
  ota::begin(DeviceName.c_str(), &display);

  energy.setup();
  energy.setSleepLimit(MAX_SLEEP_DURATION_MS);

  // TODO read last known values from eeprom/file system
  // TODO sensor calibration

  powersensors.setup();

  display.setScreen(e_screen::home);
}

void loop() {
  if (connectionHandler::isConnected())
  {
    ota::handle();

    if (ota::isUpdating())
    {
      // don't do anything else (ota handles the oled display)
      yield();
      return;
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

  connectionHandler::loop();
  switcher::loop();
  nvmem.loop();
  energy.loop(0);
  powersensors.loop();
  battery.loop();
  display.loop();
}