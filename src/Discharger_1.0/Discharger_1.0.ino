#include <LiquidCrystal_I2C.h>

#include "Constants.h"
#include "Types.h"

LiquidCrystal_I2C g_lcd(0x27, 16, 2);

Battery g_battaries[] = {
    {10, A0}, // discharge pin, adc_pin
    {9, A1},
    {8, A2},
    {7, A3},
    {6, A6},
    {5, A7}
};


const uint8_t BATTERIES_COUNT = sizeof(g_battaries) / sizeof(g_battaries[0]);

uint8_t g_screen_page = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
    pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);

    for (const auto &battery : g_battaries) {
        pinMode(battery.discharge_pin, OUTPUT);
        digitalWrite(battery.discharge_pin, HIGH);
    }

    g_lcd.init();
    g_lcd.backlight();

    Serial.println("1,2,3,4,5,6");
}

void measure_voltage();
void calculate_capacity();
void handle_relays();
void refresh_lcd();
void check_buttons();
void handle_state();

inline void lcd_clear_row();

void task_100_ms()
{
    calculate_capacity();
    handle_state();
    measure_voltage();
}

void task_20_ms()
{
    refresh_lcd();
    check_buttons();
}



void loop()
{
//    g_lcd.setCursor(0, 0);
//    g_lcd.print(String("Hello"));

//    for (auto &battery : g_battaries) {
//        auto value = analogRead(battery.adc_pin);
//        Serial.print(String(value * V_REF / 1024) + ",");
//        delay(200);

////        static uint8_t counter = 0;
////        auto &bat = g_battaries[counter % BATTERIES_COUNT];
////        const uint8_t relay = (bat.state == BatteryState::DISCHARGING) ? LOW : HIGH;
////        bat.state = (bat.state == BatteryState::DISCHARGING) ? BatteryState::STANDBY
////                                                             : BatteryState::DISCHARGING;
////        digitalWrite(bat.discharge_pin, relay);
////        counter++;

//    }
//    Serial.print("\n");

    static unsigned long last_timepoint = millis();
    unsigned long cur_timepoint = millis();

    static unsigned long accum_100ms = 0;
    accum_100ms += cur_timepoint - last_timepoint;
    if (accum_100ms >= 300) {
        task_100_ms();
        accum_100ms = 0;
    }

    task_20_ms();
    delay(20);
    last_timepoint = cur_timepoint;
}

void calculate_capacity()
{
    for (auto &battery : g_battaries) {
//        int32_t accum = 0;
//        for (const auto &value : battery.raw_voltage) {
//            accum += value;
//        }
        battery.voltage_V = static_cast<double>(battery.raw_voltage[0]) * V_REF / 1024;

        if (battery.state == BatteryState::DISCHARGING) {
            battery.current_mA = static_cast<int16_t>(battery.voltage_V / SHUNT_RES * 1000);
        } else {
            battery.current_mA = 0;
        }

        tymepoint_t now_ms = millis();

        if (battery.mesurements_tp_ms != 0) {
            double time_passed_h = static_cast<double>(now_ms - battery.mesurements_tp_ms)
                                   / 3600000;
            battery.capacity_mAh += battery.current_mA * time_passed_h;
        }
        if (battery.state == BatteryState::DISCHARGING) {
            battery.total_time_ms += now_ms - battery.mesurements_tp_ms;
        }
        battery.mesurements_tp_ms = now_ms;
    }
}

void measure_voltage()
{
    static uint8_t bat_num = 0;
    auto &battery = g_battaries[bat_num++];
    if (bat_num == BATTERIES_COUNT) {
        bat_num = 0;
    }
    static unsigned long last_timepoint = millis();
    unsigned long current_tp = millis();
    Serial.println("measure_voltage: " + String(current_tp - last_timepoint));
    last_timepoint = current_tp;
//    for (uint8_t i = 0; i < Battery::SAMPLES_SIZE; i++) {
    battery.raw_voltage[0] = analogRead(battery.adc_pin);
//        delay(20);
//    }
//    uint8_t cyclic_index = 0;

//    cyclic_index = (cyclic_index == Battery::SAMPLES_SIZE - 1) ? 0 : cyclic_index + 1;
//    for (auto &battery : g_battaries) {
//        battery.raw_voltage[cyclic_index] = analogRead(battery.adc_pin);
//    }
//    cyclic_index = (cyclic_index == Battery::SAMPLES_SIZE - 1) ? 0 : cyclic_index + 1;
}

void handle_relays()
{
    for (const auto &battery : g_battaries) {
        switch (battery.state) {
        case BatteryState::STANDBY:
            Serial.println("HIGH to Pin " + String(battery.discharge_pin));
            digitalWrite(battery.discharge_pin, HIGH);
            break;
        case BatteryState::DISCHARGING:
            Serial.println("LOW to Pin " + String(battery.discharge_pin));
            digitalWrite(battery.discharge_pin, LOW);
            break;
        }
    }
}

void print_log(const Battery &battery)
{
//    Serial.print(String(battery.voltage_V) + ", " +
//                 String(battery.current_mA) + ", " +
//                 battery.capacity_mAh + ", " +
//                 battery.total_time_ms + ", " +
//                 static_cast<uint8_t>(battery.state) + ", ");
}

void refresh_lcd()
{
    for (const auto &bat : g_battaries) {
        print_log(bat);
    }
//    Serial.print("\n");

    static auto prev_page = g_screen_page;
    if (prev_page != g_screen_page) {
        g_lcd.clear();
    }
    const auto &battery = g_battaries[g_screen_page];

    g_lcd.setCursor(0, 0);
    g_lcd.print(String(static_cast<int>(g_screen_page + 1)));
    g_lcd.print(" ");
    g_lcd.print(toString(battery.state));
    g_lcd.print(" ");
    g_lcd.print((String(battery.capacity_mAh) + "mAh").c_str());

    g_lcd.setCursor(0, 1);

    static bool display_voltage = true;
    static unsigned long tp = millis();
//    if (millis() - tp > 2000) {
//        tp = millis();
//        display_voltage = !display_voltage;
//        lcd_clear_row();
//    }

    if (display_voltage) {
        g_lcd.print((String(battery.voltage_V) + "V").c_str());
        g_lcd.print(" ");
        g_lcd.print(String(battery.current_mA) + "mA");
    } else {
        char buf[17] = {0};
        int seconds = static_cast<int>(battery.total_time_ms / 1000);
        int minutes = seconds / 60;
        int hours = minutes / 60;
        minutes = minutes % 60;
        seconds = seconds % 60;
        sprintf(buf, "    %02d:%02d:%02d    ", hours, minutes, seconds);
        g_lcd.print(buf);
    }
    prev_page = g_screen_page;
}

void check_buttons()
{
    if (BATTERIES_COUNT <= 1) {
        return;
    }

    static bool left_prev_state = LOW;
    static bool right_prev_state = LOW;
    bool left_state = digitalRead(LEFT_BUTTON_PIN);
    bool right_state = digitalRead(RIGHT_BUTTON_PIN);

    if (left_state != left_prev_state) {
        if (left_state == HIGH) {
            if (g_screen_page == 0) {
                g_screen_page = BATTERIES_COUNT - 1;
            } else {
                g_screen_page--;
            }
        }
        left_prev_state = left_state;
    }

    if (right_state != right_prev_state) {
        if (right_state == HIGH) {
            if (g_screen_page == BATTERIES_COUNT - 1) {
                g_screen_page = 0;
            } else {
                g_screen_page++;
            }
        }
        right_prev_state = right_state;
    }
}

void handle_state()
{
    for (auto &battery : g_battaries) {
        switch (battery.state) {
        case BatteryState::STANDBY:
            if (battery.voltage_V > VOLTAGE_LOW_CUT + VOLTAGE_HYSTERESIS) {
                battery.state = BatteryState::DISCHARGING;
                handle_relays();
            }
            break;
        case BatteryState::DISCHARGING:
            if (battery.voltage_V < VOLTAGE_LOW_CUT) {
                battery.state = BatteryState::STANDBY;
                handle_relays();
            }
            break;
        }
    }
}

inline void lcd_clear_row()
{
    g_lcd.print("                ");
}
