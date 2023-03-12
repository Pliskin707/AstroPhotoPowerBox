#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include "non_copy_class.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"
#include "communication_types.hpp"
#include "oled/oled.hpp"

class communication : private nonCopyable
{
    private:
        WiFiClient _client;
        MDNSResponder::hMDNSService _service;
        WiFiUDP _udp;
        struct
        {
            IPAddress IP;
            uint16_t port;
        } _RemoteAddress;
        rxContent _rxContent;
        txContent _txContent;
        DynamicJsonDocument _doc = DynamicJsonDocument(1024);
        DeserializationError _jsonError;
        bool _hasNewData = false;
        uint32_t _lastRxTime = 0;

        void _configureService (void);
        bool _read (void);
        void _send (void);

    public:
        void setup (void);
        void loop (void);
        bool hasNewData (void);
        const rxContent& data (void) const {return _rxContent;};
        void transmit (void);   // transmits the status only if a command was received or the button was pressed
        uint32_t getMsSinceLastRx (void) const;

        // for debugging
        String printLastRemoteAddress (void) {return String(_RemoteAddress.IP.toString() + ':' + String(_RemoteAddress.port));};
};

extern communication comm;  // global communication instance

#endif // __COMMUNICATION_H__