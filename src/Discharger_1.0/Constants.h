#ifndef CONSTANTS_H
#define CONSTANTS_H

const uint8_t LEFT_BUTTON_PIN = 4;
const uint8_t RIGHT_BUTTON_PIN = 3;

const double V_REF = 5.09 * 1.1; // 1.1 - voltage divider 10kOm/100kOm
const int SHUNT_RES = 10;

const double VOLTAGE_HIGH_CUT = 4.1;
const double VOLTAGE_LOW_CUT = 3.0;
const double VOLTAGE_HYSTERESIS = 0.4;
#endif // CONSTANTS_H
