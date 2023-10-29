#include "battery.hpp"

static const float _chargeTotal         = ((CAPACITY_KWH) * 3600.0f);               // [As]
static const int64_t _chargeTotal_fine  = (int64_t) (_chargeTotal * 1000000.0f);    // [µAs]
static const float _chargePerPercent    = ((_chargeTotal) / 100.0f);                // [As/%]

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
        const int32_t currentMilliAmps = (int32_t) (powersensors.getCurrent(e_psens_ch1_battery) * 1000.0f);
        const uint32_t sysTime = millis();
        const bool chargerConnected = ((currentMilliAmps > 0.0f) || (voltage > 14.2f));
        const bool idleCurrentTimeAchieved = ((sysTime - _idleCurrentSince) > 300000);
        const bool lowVoltage = (voltage <= 12.8f);
        const bool getSoCFromVoltage = lowVoltage || ((voltage >= 13.1f) && idleCurrentTimeAchieved);

        if (chargerConnected || (abs(currentMilliAmps) > 50))
            _idleCurrentSince = sysTime;

        if (_lastUpdate)   // already initialized?
        {
            // calculate the area (= capacity) between the previous and actual power reading with linear interpolation
            const int32_t deltaTimeMs = (sysTime - _lastUpdate);
            const int32_t currentSumMilliAmps = currentMilliAmps + _prevCurrentMilliAmps;
            const int32_t deltaCharge = (currentSumMilliAmps * deltaTimeMs) / 2;    // [A * 1000] * [s * 1000] = [Aµs]

            // update the known capacity
            _chargeRemaining_fine += deltaCharge;
            _chargeRemaining_fine = min(_chargeRemaining_fine, _chargeTotal_fine);
            _chargeRemaining_fine = max(_chargeRemaining_fine, 0LL);
            _chargeRemaining = ((float) _chargeRemaining_fine) / 1000000.0f;
            nvmem.setRemainingCharge(_chargeRemaining);

            // update the state of charge
            float SoCfromCharge = _chargeRemaining / _chargePerPercent;
            SoCfromCharge = fminf(100.0f, fmaxf(0.0f, SoCfromCharge));
            float SoCfromVoltage = _getSoCFromRestingVoltage(voltage);
            SoCfromVoltage = fminf(100.0f, fmaxf(0.0f, SoCfromVoltage));
            float SoCdiff = fabsf(SoCfromVoltage - SoCfromCharge);

            if ((SoCdiff > 90.0f) || ((SoCfromCharge < 10.0f) && !lowVoltage) ||    // desync (maybe file broken or charged when the controller was not running)
                ((SoCdiff > 10.0f) && getSoCFromVoltage))                           // plausibility
            {
                _SoC = SoCfromVoltage;
                _SoCgood = false;

                if (idleCurrentTimeAchieved)
                    _chargeRemaining_fine = (int64_t) (((float) _chargeTotal_fine) * SoCfromVoltage / 100.0f);
            }
            else
            {
                _SoC = SoCfromCharge;
                _SoCgood = true;
            }


            // "brown out" detection
            // TODO if the supply voltage drops / gets critical, store the values in EEPROM

        }

        _lastUpdate           = sysTime;
        _prevCurrentMilliAmps = currentMilliAmps;
    }
}

float lifepo4_battery::getEnergyRemainingWh(void) const
{
    return _chargeRemaining * (1.0f/3600.0f) * restingVoltageCurve[2].totalVoltage;    // ([As] -> [Ah]) * nominal batVoltage
}

float lifepo4_battery::getCapacityTotal(void) const 
{
    return _chargeTotal;
}

void lifepo4_battery::_initFromMemory(void)
{
    _initialized = true;

    const auto &prevStats = nvmem.getStats();
    _chargeRemaining = prevStats.remainingCharge;
    _chargeRemaining_fine = ((uint64_t) _chargeRemaining) * 1000000;    // As -> Aµs
}
