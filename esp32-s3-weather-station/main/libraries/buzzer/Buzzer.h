// Copyright (c) 2025 chendi
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT
#pragma once
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <cstdint>
#include <esp_err.h>
#include <memory>
#include <vector>

struct BuzzerNote
{
    int frequency = 4000;
    int duration_ms = 500;
    float volume = 0.1f; // avoid too loud, range: 0.0f ~ 1.0f
};

using BuzzerMusic = std::vector<BuzzerNote>;
using BuzzerMusicPtr = std::shared_ptr<BuzzerMusic>;
using MusicLength_t = uint32_t;

class Buzzer
{
  private:
    std::shared_ptr<struct BuzzerPImpl> Pimpl = nullptr;

  public:
    Buzzer();
    virtual ~Buzzer();

    /**
     * @brief Init buzzer gpio
     *
     * @param gpio buzzer gpio number
     * @param gyro_sensitivity gyroscope sensitivity
     *
     * @return
     *     - ESP_OK Success
     *     - ESP_FAIL Fail
     */
    esp_err_t Init(const gpio_num_t _gpio_num, ledc_clk_cfg_t _clk_cfg = LEDC_AUTO_CLK,
                   ledc_mode_t _speed_mode = LEDC_LOW_SPEED_MODE, ledc_timer_bit_t _timer_bit = LEDC_TIMER_13_BIT,
                   ledc_timer_t _timer_num = LEDC_TIMER_0, ledc_channel_t _ledc_channel = LEDC_CHANNEL_0,
                   uint32_t _idle_level = 0);

    /**
     * @brief Release buzzer gpio
     *
     * @return
     *     - ESP_OK Success
     *     - ESP_FAIL Fail
     */
    esp_err_t Deinit();

    void Beep(const BuzzerNote &note);
    void Play(const BuzzerMusic &music);
    void Play(BuzzerMusic &&music);
    void Play(BuzzerMusicPtr music);
    void Stop();
};
