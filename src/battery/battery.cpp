#include "battery.hpp"

lifepo4_battery battery;    // global instance

typedef struct
{
    float totalVoltage;
    float stateOfCharge;
} t_bat_lut;

// see https://cleversolarpower.com/lifepo4-voltage-chart/ 
//! must be ordered ascending by voltage
static const t_bat_lut restingVoltageCurve[] PROGMEM = 
{
    {10.0,  0},
    {12.0, 10},
    {12.8, 20},
    {12.9, 30},
    {13.0, 40},
    {13.0, 50},
    {13.1, 60},
    {13.2, 70},
    {13.3, 80},
    {13.4, 90},
    {13.6, 100}    
};

static float _getSoCFromRestingVoltage (const float restingBatVoltage)
{
    uint_fast8_t numEntries = sizeof(restingVoltageCurve) / sizeof(restingVoltageCurve[0]);
    t_bat_lut entry, prevEntry;
    float SoC = 0.0f;

    // check end first
    memcpy_P(&entry, &(restingVoltageCurve[numEntries - 1]), sizeof(entry));
    if (restingBatVoltage >= entry.totalVoltage)
        return entry.stateOfCharge;

    memcpy_P(&entry, &(restingVoltageCurve[0]), sizeof(entry));
    if (restingBatVoltage <= entry.totalVoltage)
        return entry.stateOfCharge;

    for (uint_fast8_t ndx = 1; ndx < numEntries; ndx++)
    {
        prevEntry = entry;
        memcpy_P(&entry, &(restingVoltageCurve[ndx]), sizeof(entry));

        if (restingBatVoltage > entry.totalVoltage)
            continue;

        SoC = prevEntry.stateOfCharge + ((entry.stateOfCharge - prevEntry.stateOfCharge) * (restingBatVoltage - prevEntry.totalVoltage) / (entry.totalVoltage - prevEntry.totalVoltage));
        break;
    }

    return SoC;
}

void lifepo4_battery::loop (void)
{
    if (!_initialized)
        _initFromMemory();

    if (millis() >= (_lastUpdate + _updateDelay))
    {
        // call this first so the (time consuming) TWI communication is handled before the time is calculated
        const float voltage = powersensors.getVoltage(e_psens_ch1_battery);
        const float current = powersensors.getCurrent(e_psens_ch1_battery);
        const uint32_t sysTime = millis();
        _idleCurrentSince = (fabsf(current) < 0.05) ? (_idleCurrentSince ? _idleCurrentSince : sysTime) : 0;
        const bool getSoCFromVoltage = ((voltage <= 12.8) || (voltage >= 13.3)) && ((sysTime - _idleCurrentSince) > 60000);

        if (_lastUpdate)   // already initialized?
        {
            // calculate the area (= capacity) between the previous and actual power reading with linear interpolation
            const float deltaTime = ((float) (sysTime - _lastUpdate)) / 1000.0f;
            const float currentSum = current + _prevCurrent;
            const float deltaCharge = (currentSum * deltaTime) / 2.0f;

            // update the known capacity
            _chargeRemaining += deltaCharge;
            nvmem.setRemainingCharge(fmaxf(_chargeRemaining, 0.0f));    // don't store negative charge values

            // update the state of charge
            if (!getSoCFromVoltage)
            {
                float actSoC = _chargeRemaining / _chargeTotal;
                actSoC = fminf(100.0f, fmaxf(0.0f, actSoC));
                // TODO plausibility with measured battery voltage
                _SoC = actSoC;
            }


            // "brown out" detection
            // TODO if the supply voltage drops / gets critical, store the values in EEPROM

        }

        // calculate SoC
        if (getSoCFromVoltage)
        {
            _SoC = _getSoCFromRestingVoltage(voltage);
            _SoCgood = true;
        }

        _lastUpdate  = sysTime;
        _prevCurrent = current;
        _prevVoltage = voltage;
    }
}

float lifepo4_battery::getEnergyRemainingWh(void) const
{
    return _chargeRemaining * (1.0f/3600.0f) * restingVoltageCurve[2].totalVoltage;    // ([As] -> [Ah]) * nominal batVoltage
}
void lifepo4_battery::_initFromMemory(void)
{
    _initialized = true;

    const auto &prevStats = nvmem.getStats();
    _chargeRemaining = prevStats.remainingCharge;
}
