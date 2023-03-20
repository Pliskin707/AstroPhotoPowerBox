#ifndef PSENS_HPP
#define PSENS_HPP

#include <memory>

#include <Wire.h>

#include "non_copy_class.hpp"
#include "ina3221.hpp"

typedef enum 
{
    e_psens_ch1_battery = 0,
    e_psens_ch2_dew_heater_1,
    e_psens_ch3_dew_heater_2,
    e_psens_ch4_pc,
    e_psens_ch5_mount,
    e_psens_ch6_imaging_cam,
    e_psens_num_channels
} e_psens_channel;


class psens : private nonCopyable
{
    private:
        std::shared_ptr<ina3221::sensor> _sensors[2];
        uint32_t _nextUpdate = 0;
        uint32_t _updateDelay = 100;
        float _powers[e_psens_num_channels] = {0.0f};

        std::shared_ptr<ina3221::sensor> _getSensorAndChannel (const e_psens_channel channel, uint_fast8_t * sensChannel = nullptr);

    public:
        explicit psens (TwoWire * wire = &Wire, const uint8_t addr1 = INA3221_ADDR_GND, const uint8_t addr2 = INA3221_ADDR_VS);
        void loop (void);
        std::shared_ptr<ina3221::sensor> getSensor (uint_fast8_t sensorIndex);
        float getVoltage (const e_psens_channel channel);
        float getCurrent (const e_psens_channel channel);
        float getPower (const e_psens_channel channel);
};

extern psens powersensors;
#endif