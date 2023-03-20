#ifndef __INA3221_TYPES__
#define __INA3221_TYPES__

#include <Arduino.h>

#define INA3221_ADDR_GND    (0b1000000)
#define INA3221_ADDR_VS     (0b1000001)
#define INA3221_ADDR_SDA    (0b1000010)
#define INA3221_ADDR_SCL    (0b1000011)

#define INA3221_NUM_CH      (3u)

namespace ina3221
{

typedef struct value
{
    float value;
    uint32_t time;
    int16_t adc;
    bool valid;
} value;

typedef struct adcCorrection
{
    float factor    = 1.0f;
    float offset    = 0.0f;
} adcCorrection;

typedef enum
{
    e_ina3221_avg_none  = 0,    // no average
    e_ina3221_avg_4     = 1,    // average over 4 samples
    e_ina3221_avg_16    = 2,
    e_ina3221_avg_64    = 3,
    e_ina3221_avg_128   = 4,
    e_ina3221_avg_256   = 5,
    e_ina3221_avg_512   = 6,
    e_ina3221_avg_1024  = 7
} avgSamples;

typedef enum
{
    e_ina3221_conv_140us    = 0,
    e_ina3221_conv_204us    = 1,
    e_ina3221_conv_332us    = 2,
    e_ina3221_conv_1100us   = 3,
    e_ina3221_conv_2116us   = 4,
    e_ina3221_conv_4156us   = 5,
    e_ina3221_conv_8224us   = 6,
    e_ina3221_conv_us       = 7
} convTime;

}

#endif