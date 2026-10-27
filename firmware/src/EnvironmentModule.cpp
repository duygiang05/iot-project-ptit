#include "EnvironmentModule.h"

EnvironmentModule::EnvironmentModule()
    : _pinWater(Pin::WATER_ADC),
      _pinLdr(Pin::LDR_ADC),
      _pinLed(Pin::LED),
      _waterThreshold(1500),
      _lightOnThreshold(1600),
      _lightOffThreshold(2100),
      _lastSampleMs(0),
      _waterAcc(0),
      _ldrAcc(0),
      _sampleCount(0) {
    _currentData.water_detected = false;
    _currentData.light_level = 0;
    _currentData.led_on = false;
    _currentData.raw_water_adc = 0;
}

void EnvironmentModule::begin() {
    pinMode(_pinWater, INPUT);
    pinMode(_pinLdr, INPUT);
    pinMode(_pinLed, OUTPUT);
    digitalWrite(_pinLed, LOW);
    _currentData.led_on = false;
}

void EnvironmentModule::setWaterThreshold(int threshold) {
    _waterThreshold = threshold;
}

void EnvironmentModule::setLightThresholds(int onThreshold, int offThreshold) {
    _lightOnThreshold = onThreshold;
    _lightOffThreshold = offThreshold;
}

void EnvironmentModule::update(unsigned long nowMs) {
    // Chỉ đọc mẫu theo chu kỳ SAMPLE_INTERVAL_MS, hoàn toàn không chặn (non-blocking)
    if (nowMs - _lastSampleMs < SAMPLE_INTERVAL_MS) {
        return;
    }
    _lastSampleMs = nowMs;

    // Đọc ADC từ chân analog
    int w = analogRead(_pinWater);
    int l = analogRead(_pinLdr);

    _waterAcc += w;
    _ldrAcc += l;
    _sampleCount++;

    if (_sampleCount >= FILTER_SAMPLES) {
        int avgWater = static_cast<int>(_waterAcc / _sampleCount);
        int avgLdr = static_cast<int>(_ldrAcc / _sampleCount);

        _waterAcc = 0;
        _ldrAcc = 0;
        _sampleCount = 0;

        _currentData.raw_water_adc = avgWater;
        _currentData.light_level = avgLdr;

        // Phán đoán chạm nước: Khi ADC > ngưỡng nước
        _currentData.water_detected = (avgWater >= _waterThreshold);

        // Phán đoán bật/tắt đèn với cơ chế trễ Hysteresis
        if (!_currentData.led_on && avgLdr < _lightOnThreshold) {
            _currentData.led_on = true;
            digitalWrite(_pinLed, HIGH);
        } else if (_currentData.led_on && avgLdr > _lightOffThreshold) {
            _currentData.led_on = false;
            digitalWrite(_pinLed, LOW);
        }
    }
}

EnvironmentData EnvironmentModule::getData() const {
    return _currentData;
}
