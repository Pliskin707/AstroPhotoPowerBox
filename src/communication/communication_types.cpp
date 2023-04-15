#include "communication_types.hpp"

void convertToJson (const txContent &src, JsonVariant dst)
{
    dst["time"]             = src.timestamp;
    dst["button pressed"]   = src.buttonPressed;
    dst["press count"]      = src.pressCount;
    dst["wifi dbm"]         = src.wifiStrength;

    dst["battery info"]     = src.batteryInfo;
    dst["dew info"]         = src.dewInfo;

    JsonObject jConsumers = dst["consumers"].as<JsonObject>();
    if (jConsumers.isNull())
        jConsumers = dst.createNestedObject("consumers");

    jConsumers["charger"]   = src.chargerInfo;
    jConsumers["pc"]        = src.pcInfo;
    jConsumers["mount"]     = src.mountInfo;
    jConsumers["camera"]    = src.cameraInfo;
    jConsumers["heater1"]   = src.heater1Info;
    jConsumers["heater2"]   = src.heater2Info;

    return;
}

void convertToJson(const consumerInfo &src, JsonVariant dst)
{
    dst["is powered"]    = src.isPowered;
    dst["is active"]     = src.isActive;
    dst["voltage"]       = src.voltage;
    dst["current"]       = src.current;
    dst["power"]         = src.power;
    dst["average power"] = src.avgPower;
}

void convertToJson(const batInfo &src, JsonVariant dst)
{
    qualityToJson(src.quality, dst);
    dst["SoC"]                  = src.SoC;
    dst["energy"]               = src.energy;
    dst["voltage"]              = src.voltage;
    dst["current"]              = src.current;
    dst["seconds to full"]      = src.chargeSecondsRemainingToFullSoC;
    dst["seconds to bulk"]      = src.chargeSecondsRemainingToBulkSoC;
    dst["seconds to empty"]     = src.dischargeSecondsRemainingToEmpty;
    dst["seconds to reserved"]  = src.dischargeSecondsRemainingToReserved;
}

void convertToJson(const dewSensorInfo &src, JsonVariant dst)
{
    qualityToJson(src.quality, dst);
    dst["temperature"]       = src.temperature;
    dst["relative humidity"] = src.relHumidity;
}

void qualityToJson(const infoQuality &src, JsonVariant &dst)
{
    switch (src)
    {
        case infoQuality::good:     dst["quality"] = "good";    break;
        case infoQuality::medium:   dst["quality"] = "medium";  break;
        default:                    dst["quality"] = "bad";     break;
    }
}


void convertFromJson (JsonVariantConst src, rxContent &dst)
{
    // reset all to default
    dst = rxContent();

    // TODO

    return;
}