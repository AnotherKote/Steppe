#ifndef TYPES_H
#define TYPES_H

using tymepoint_t = unsigned long;

enum class BatteryState { STANDBY, DISCHARGING };

const char *toString(BatteryState s)
{
    switch (s) {
    case BatteryState::STANDBY:
        return "STNDB";
        break;
    case BatteryState::DISCHARGING:
        return "DCHRG";
        break;
    default:
        return "UNKWN";
        break;
    }
}

struct Battery
{
    static const uint8_t SAMPLES_SIZE = 1;

    double voltage_V = 0.0;
    int raw_voltage[SAMPLES_SIZE] = {0};
    int16_t current_mA = 0;
    double capacity_mAh = 0.0;
    tymepoint_t total_time_ms = 0;
    BatteryState state = BatteryState::STANDBY;
    tymepoint_t mesurements_tp_ms = 0;

    const uint8_t discharge_pin;
    const uint8_t adc_pin;

    Battery(uint8_t _discharge_pin, uint8_t _adc_pin)
        : discharge_pin(_discharge_pin), adc_pin(_adc_pin)
    {}
};

#endif // TYPES_H
