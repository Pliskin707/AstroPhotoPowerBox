#include "communication_types.hpp"

void convertToJson (const txContent &src, JsonVariant dst)
{
    dst["time"]             = src.timestamp;
    dst["button pressed"]   = src.buttonPressed;
    dst["press count"]      = src.pressCount;
    // TODO

    return;
}


void convertFromJson (JsonVariantConst src, rxContent &dst)
{
    // reset all to default
    dst = rxContent();

    // TODO

    return;
}