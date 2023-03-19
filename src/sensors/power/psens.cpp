// #include "psens.hpp"

// psens powersensors;

// psens::psens (TwoWire * wire, const ina3221_addr_t addr1, const ina3221_addr_t addr2) : 
//     _sensors({std::unique_ptr<INA3221>(new INA3221(addr1)), std::unique_ptr<INA3221>(new INA3221(addr2))})
// {
//     for (const auto &sensor : _sensors)
//     {
//         sensor->begin(wire);
//         sensor->reset();
//         sensor->setShuntRes(12, 100, 100);  // first channel has a shunt mod on both sensors
//     }
// }

// void psens::loop (void)
// {
//     const uint32_t sysTime = millis();
//     if (sysTime >= _nextUpdate)
//     {
//         _nextUpdate = sysTime + _updateDelay;

//         const ina3221_ch_t channels[] = {INA3221_CH1, INA3221_CH2, INA3221_CH3};
//         uint_fast8_t ii = 0;
//         for (const auto &sensor : _sensors)
//         {
//             for (const auto &channel : channels)
//             {
//                 _voltages[ii] = sensor->getVoltage(channel);
//                 _currents[ii] = sensor->getCurrent(channel);
//                 ii++;
//             }
//         }
//     }
// }