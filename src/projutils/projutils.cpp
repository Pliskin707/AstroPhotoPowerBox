#include "projutils.hpp"
#include "config.hpp"

ClassDeviceName::ClassDeviceName()
{
    char name[20];
    uint8 mac[6];

    wifi_get_macaddr(STATION_IF, mac);
    snprintf_P(name, sizeof(name), PSTR("%s_%02X%02X%02X"), DEVICE_PREFIX, mac[3], mac[4], mac[5]);
    name[sizeof(name) - 1] = 0x00;  // guarantee End-Of-String

    this->concat(name);
}
