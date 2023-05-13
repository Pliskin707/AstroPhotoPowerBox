#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "config.hpp"
#include "projutils/projutils.hpp"
#include "wifiauth2.h"

/* THIS REQUIRES A MULTI_AP LIST TO BE DEFINED LIKE THIS:

const struct 
{
    const char ssid[20];        // <- adjust the length as required
    const char password[30];
} ssid_list[] =
{
    {"SSID_1", "PASSWORD_1"},
    {"SSID_2", "PASSWORD_2"},
    ...
    {"SAME_NETWORK_SSID_THE_PC_WILL_JOIN", "PASSWORD_PC_SSID"}   // this one needs to be the last
};

const uint8_t ssid_list_len = sizeof(ssid_list) / sizeof(ssid_list[0]);
*/

// handles the wifi transceiver
namespace connectionHandler
{
    void setup (void);
    void loop (void);
    bool isConnected (void);
    bool isInternet (void);
    uint8_t getSSIDndx (void);  // TODO use this to display a number next to the status bar symbol for RSSI
};

#endif // __CONNECTION_H__