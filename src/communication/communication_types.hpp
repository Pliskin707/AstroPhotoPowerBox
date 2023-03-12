#ifndef __COMMUNICATION_TYPES_HPP__
#define __COMMUNICATION_TYPES_HPP__

#include <stdlib.h>
#include <vector>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.hpp"

#define MAXTEXTLEN  (40)

typedef enum {bad, medium, good} infoQuality;

typedef struct
{
    infoQuality quality = bad;
    float SoC           = 0.0f;
    float energy        = 0.0f;
    float voltage       = 0.0f;
    float current       = 0.0f;
    bool isCharging     = false;
    uint32_t chargeSecondsRemainingToFullSoC        = 0;
    uint32_t chargeSecondsRemainingToBulkSoC        = 0;
    uint32_t dischargeSecondsRemainingToEmpty       = 0;
    uint32_t dischargeSecondsRemainingToReserved    = 0;
} batInfo;

typedef struct
{
    bool isPowered      = false;
    bool isActive       = false;
    float voltage       = 0.0f;
    float current       = 0.0f;
    float power         = 0.0f;
    float avgPower      = 0.0f;
} consumerInfo;

typedef struct
{
    infoQuality quality = bad;
    float temperature   = 0.0f;
    float relHumidity   = 0.0f;
} dewSensorInfo;

typedef struct 
{
    uint32_t timestamp  = 0;
    uint32_t pressCount = 0;
    bool buttonPressed  = false;
    int8_t wifiStrength = 0;

    batInfo batteryInfo;
    dewSensorInfo dewInfo;
    consumerInfo chargerInfo;
    consumerInfo pcInfo;
    consumerInfo mountInfo;
    consumerInfo cameraInfo;
    consumerInfo heater1Info;
    consumerInfo heater2Info;    
} txContent;

typedef struct
{

} rxContent;

// these are custom converters for ArduinoJson
void convertToJson (const txContent &src, JsonVariant dst);
void convertToJson (const consumerInfo &src, JsonVariant dst);
void convertFromJson (JsonVariantConst src, rxContent &dst);

#endif