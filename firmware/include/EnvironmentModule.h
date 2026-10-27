#pragma once

#include <Arduino.h>
#include "PinConfig.h"

/**
 * @brief DTO chứa dữ liệu môi trường theo đúng hợp đồng docs/MODULE_INTERFACE.md
 */
struct EnvironmentData {
    bool water_detected;   // true: phát hiện chạm nước, false: khô ráo
    int light_level;       // Mức sáng ADC (0 - 4095)
    bool led_on;           // Trạng thái đèn LED ban đêm (true: bật, false: tắt)
    int raw_water_adc;     // Giá trị ADC thực tế của cảm biến nước (hỗ trợ debug)
};

/**
 * @brief Module Môi Trường & Đèn (Do Đạt phụ trách)
 * Quản lý cảm biến nước, cảm biến ánh sáng LDR và đèn LED an toàn ban đêm.
 * Thiết kế Non-blocking, có bộ lọc làm mượt ADC và Hysteresis chống nhiễu đèn.
 */
class EnvironmentModule {
public:
    EnvironmentModule();

    /**
     * @brief Khởi tạo các chân GPIO cho cảm biến và LED
     */
    void begin();

    /**
     * @brief Cập nhật trạng thái module định kỳ (Non-blocking)
     * @param nowMs Thời gian hiện tại lấy từ millis()
     */
    void update(unsigned long nowMs);

    /**
     * @brief Lấy snapshot dữ liệu môi trường hiện tại
     * @return EnvironmentData 
     */
    EnvironmentData getData() const;

    // Các hàm cấu hình ngưỡng (owner Đạt có thể tinh chỉnh)
    void setWaterThreshold(int threshold);
    void setLightThresholds(int onThreshold, int offThreshold);

private:
    int _pinWater;
    int _pinLdr;
    int _pinLed;

    // Ngưỡng phát hiện
    int _waterThreshold;
    int _lightOnThreshold;
    int _lightOffThreshold;

    // Dữ liệu nội bộ
    EnvironmentData _currentData;

    // Quản lý thời gian đọc cảm biến
    unsigned long _lastSampleMs;
    static constexpr unsigned long SAMPLE_INTERVAL_MS = 100;

    // Bộ lọc trung bình trượt (Moving average filter)
    static constexpr int FILTER_SAMPLES = 5;
    long _waterAcc;
    long _ldrAcc;
    int _sampleCount;
};
