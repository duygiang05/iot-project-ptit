
#pragma once

// Smart Cane IoT - GPIO configuration
// ESP32 DevKitC V4
// Proposed G09 pinout - validate against real wiring.

namespace Pin {

  // HC-SR04: obstacle detection
  constexpr int HC_FRONT_TRIG = 25;
  constexpr int HC_FRONT_ECHO = 26;

  // HC-SR04: downward detection
  constexpr int HC_DOWN_TRIG = 18;
  constexpr int HC_DOWN_ECHO = 19;

  // Vibration motor driver (PWM)
  constexpr int MOTOR_DRIVER = 27;

  // GPS NEO-6M (UART2)
  constexpr int GPS_RX = 16;
  constexpr int GPS_TX = 17;

  // MPU6050 (I2C)
  constexpr int MPU_SDA = 21;
  constexpr int MPU_SCL = 22;

  // Emergency button and buzzer
  constexpr int SOS_BUTTON = 13;
  constexpr int BUZZER_DRIVER = 14;

  // Environment sensors
  constexpr int WATER_ADC = 35;
  constexpr int LDR_DO = 34;

  // Night LED
  constexpr int LED = 32;

}
