#include "connection.hpp"
#include "communication/communication.hpp"
#include "oled/oled.hpp"

namespace connectionHandler {

static int_fast8_t _authNdx = -1;
static uint32_t _disconnectedSince = 0;
static uint32_t _connectedSince = 0;
static WiFiEventHandler _eventHandler[2];
static bool _gotIp = false;
static bool _telegramStarted = false;
static bool _mdnsStarted = false;

static void _tryNextSSID (void)
{

    _authNdx++;
    _authNdx %= ssid_list_len;
    const auto& network = ssid_list[_authNdx];

    _disconnectedSince  = millis();
    _connectedSince     = 0;
    WiFi.begin(network.ssid, network.password);
    dprintf("Trying network \"%s\"\n", network.ssid);
}

void setup(void)
{
    // reset
    _authNdx = -1;

    // Wifi
    WiFi.setAutoConnect(false);
    WiFi.hostname(DeviceName);
    WiFi.mode(WIFI_STA);

    _eventHandler[0] = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &info)
    {
        _gotIp = true; 
        _connectedSince = millis();
        _disconnectedSince = 0;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.centerText("Got IP:");
        display.setCursor(0, 8);
        display.centerText(info.ip.toString());
        display.suppressContentTemporary(3000);
        //dprintf("Got IP: %s\n", info.ip.toString().c_str());
    });

    _eventHandler[1] = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &info)
    {
        _gotIp = false; 
        _connectedSince = 0;
        _telegramStarted = false;
        _disconnectedSince = millis();
        telegramBot.reset();
        WiFi.reconnect();
        display.clearDisplay();
        display.showWarning("WiFi lost");
        display.suppressContentTemporary(3000);
        //dprintf("Disconnected (%d)\n", info.reason);
    });       
}

void loop(void)
{
    if (isConnected())
    {
        if (!_mdnsStarted)
        {
            if (MDNS.begin(DeviceName))
            {
                comm.setup();
                _mdnsStarted = true;
            }
        }
        else
            MDNS.update();

        if (!_telegramStarted)
            _telegramStarted = telegramBot.begin();
    }
    else if ((millis() - _disconnectedSince) > 10000)
    {
        _tryNextSSID();
    }
}

bool isConnected(void)
{
    return _gotIp;
}

uint8_t getSSIDndx(void)
{
    return _authNdx;
}

}