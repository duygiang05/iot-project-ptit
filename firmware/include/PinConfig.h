#pragma once
// Draft: check real wiring and docs/PINOUT.md before enabling hardware.
namespace Pin {
constexpr int HC_FRONT_TRIG = 25;
constexpr int HC_FRONT_ECHO = 26;
constexpr int MOTOR_DRIVER = 27;
constexpr int GPS_RX = 16;
constexpr int GPS_TX = 17;
constexpr int MPU_SDA = 21;
constexpr int MPU_SCL = 22;
constexpr int SOS_BUTTON = 13;
constexpr int BUZZER_DRIVER = 14;
constexpr int WATER_ADC = 35;
constexpr int LDR_ADC = 34;
constexpr int LED = 32;
constexpr int HC_DOWN_TRIG = 18;
constexpr int HC_DOWN_ECHO = 19;
}
