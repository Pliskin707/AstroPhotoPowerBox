// #ifndef PSENS_HPP
// #define PSENS_HPP

// #include <memory>

// #include <Wire.h>

// #include "non_copy_class.hpp"

// typedef enum 
// {
//     e_psens_ch1_battery = 0,
//     e_psens_ch2_dew_heater_1,
//     e_psens_ch3_dew_heater_2,
//     e_psens_ch4_pc,
//     e_psens_ch5_mount,
//     e_psens_ch6_imaging_cam,
//     e_psens_num_channels
// } e_psens_channel;


// // class psens : private nonCopyable
// // {
// //     private:
// //         std::unique_ptr<INA3221> _sensors[2];   // Sensor1: measures battery, dew heater 1 and dew heater 2; Sensor2: measures pc, mount and imaging camera
// //         uint32_t _nextUpdate = 0;
// //         uint32_t _updateDelay = 100;
// //         float _voltages[e_psens_num_channels];
// //         float _currents[e_psens_num_channels];

// //     public:
// //         explicit psens (TwoWire * wire = &Wire, const ina3221_addr_t addr1 = INA3221_ADDR40_GND, const ina3221_addr_t addr2 = INA3221_ADDR41_VCC);
// //         void loop (void);
// // };

// // extern psens powersensors;
// #endif