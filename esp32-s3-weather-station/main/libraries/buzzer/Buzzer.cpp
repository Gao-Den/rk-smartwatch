// Copyright (c) 2025 chendi
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "Buzzer.h"
#include "HAL/ledc_ll.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <algorithm>

static const char *TAG = "buzzer";
static const uint32_t StackSize = 2048;

#define CHECK_PIMPL()                                                                                                  \
    if (this->Pimpl == nullptr)                                                                                        \
    {                                                                                                                  \
        ESP_LOGE(TAG, "Please init buzzer first");                                                                     \
        return;                                                                                                        \
    }

struct BuzzerPImpl
{
    TaskHandle_t TaskHandle = nullptr;
    SemaphoreHandle_t SemaphoreHandle = nullptr;
    ledc_mode_t ledc_speed_mode;
    ledc_timer_bit_t ledc_timer_bit;
    ledc_timer_t ledc_timer_num;
    ledc_channel_t ledc_channel;
    uint32_t ledc_idle_level;

    ~BuzzerPImpl()
    {
        if (TaskHandle != nullptr)
        {
            vTaskDelete(TaskHandle);
        }
        if (SemaphoreHandle != nullptr)
        {
            vSemaphoreDelete(SemaphoreHandle);
        }
        StopBuzzer();
    }

    void Task()
    {
        while (true)
        {
            if (xSemaphoreTake(SemaphoreHandle, portMAX_DELAY) == pdTRUE)
            {
                bool NewTask = true;
                while (NewTask)
                {
                    NewTask = false;
                    BuzzerMusicPtr playing = music;
                    if (playing != nullptr)
                    {
                        for (auto &note : *playing)
                        {
                            SetBuzzerNote(note.frequency, note.volume);
                            if (xSemaphoreTake(SemaphoreHandle, pdMS_TO_TICKS(note.duration_ms)) == pdTRUE)
                            {
                                NewTask = true;
                                break;
                            }
                        }
                    }
                    StopBuzzer();
                }
            }
        }
    }

    void SetBuzzerNote(uint32_t frequency, float volume)
    {
        frequency = std::clamp(frequency, 1ul << LEDC_LL_FRACTIONAL_BITS, 1ul << ledc_timer_bit);
        volume = std::clamp(volume, 0.0f, 1.0f);
        uint32_t duty = (1 << ledc_timer_bit) * volume;
        ESP_ERROR_CHECK(ledc_set_duty(ledc_speed_mode, ledc_channel, duty));
        ESP_ERROR_CHECK(ledc_update_duty(ledc_speed_mode, ledc_channel));
        ESP_ERROR_CHECK(ledc_set_freq(ledc_speed_mode, ledc_timer_num, frequency));
    }

    void StopBuzzer()
    {
        ESP_ERROR_CHECK(ledc_set_duty(ledc_speed_mode, ledc_channel, 0));
        ESP_ERROR_CHECK(ledc_update_duty(ledc_speed_mode, ledc_channel));
        ESP_ERROR_CHECK(ledc_stop(ledc_speed_mode, ledc_channel, ledc_idle_level));
    }

    static void buzzer_task(void *pvParameters)
    {
        BuzzerPImpl *pimpl = static_cast<BuzzerPImpl *>(pvParameters);
        pimpl->Task();
    }

    esp_err_t Init(const gpio_num_t _gpio_num, ledc_clk_cfg_t _clk_cfg, ledc_mode_t _speed_mode,
                   ledc_timer_bit_t _timer_bit, ledc_timer_t _timer_num, ledc_channel_t _ledc_channel,
                   uint32_t _idle_level)
    {

        ledc_speed_mode = _speed_mode;
        ledc_timer_bit = _timer_bit;
        ledc_timer_num = _timer_num;
        ledc_channel = _ledc_channel;
        ledc_idle_level = _idle_level;

        ledc_timer_config_t ledc_timer = {
            .speed_mode = ledc_speed_mode,
            .duty_resolution = ledc_timer_bit,
            .timer_num = ledc_timer_num,
            .freq_hz = 4000,
            .clk_cfg = _clk_cfg,
            .deconfigure = false,
        };
        ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

        ledc_channel_config_t channel_config = {.gpio_num = _gpio_num,
                                                .speed_mode = ledc_speed_mode,
                                                .channel = ledc_channel,
                                                .intr_type = LEDC_INTR_DISABLE,
                                                .timer_sel = ledc_timer_num,
                                                .duty = 0,
                                                .hpoint = 0,
                                                .flags = {}};
        ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
        ESP_ERROR_CHECK(ledc_stop(ledc_speed_mode, ledc_channel, ledc_idle_level));

        SemaphoreHandle = xSemaphoreCreateBinary();
        BaseType_t res =
            xTaskCreate(buzzer_task, "buzzer_task", StackSize, this, uxTaskPriorityGet(nullptr), &TaskHandle);

        if (res == pdPASS)
        {
            return ESP_OK;
        }
        else
        {
            return ESP_FAIL;
        }
    }

    void Play(BuzzerMusicPtr music)
    {
        this->music = music;
        xSemaphoreGive(SemaphoreHandle);
    }

    BuzzerMusicPtr music;
};

Buzzer::Buzzer()
{
}

Buzzer::~Buzzer()
{
    Deinit();
}

esp_err_t Buzzer::Init(const gpio_num_t _gpio_num, ledc_clk_cfg_t _clk_cfg, ledc_mode_t _speed_mode,
                       ledc_timer_bit_t _timer_bit, ledc_timer_t _timer_num, ledc_channel_t _ledc_channel,
                       uint32_t _idle_level)
{
    if (this->Pimpl != nullptr)
    {
        ESP_LOGE(TAG, "Buzzer has been initialized, deinit before init again.");
        return ESP_FAIL;
    }

    auto impl = std::make_shared<BuzzerPImpl>();
    esp_err_t res = impl->Init(_gpio_num, _clk_cfg, _speed_mode, _timer_bit, _timer_num, _ledc_channel, _idle_level);

    if (res == ESP_OK)
    {
        Pimpl = impl;
    }
    return res;
}

esp_err_t Buzzer::Deinit()
{
    if (this->Pimpl != nullptr)
    {
        this->Pimpl = nullptr;
    }
    return ESP_OK;
}

void Buzzer::Beep(const BuzzerNote &note)
{
    CHECK_PIMPL();

    BuzzerMusicPtr NewMusic = std::make_shared<BuzzerMusic>();
    NewMusic->push_back(note);
    Play(NewMusic);
}

void Buzzer::Play(const BuzzerMusic &music)
{
    CHECK_PIMPL();
    BuzzerMusicPtr NewMusic = std::make_shared<BuzzerMusic>();
    *NewMusic = music;
    Play(NewMusic);
}

void Buzzer::Play(BuzzerMusic &&music)
{
    CHECK_PIMPL();
    BuzzerMusicPtr NewMusic = std::make_shared<BuzzerMusic>();
    *NewMusic = std::move(music);
    Play(NewMusic);
}

void Buzzer::Play(BuzzerMusicPtr music)
{
    CHECK_PIMPL();
    Pimpl->Play(music);
}

void Buzzer::Stop()
{
    CHECK_PIMPL();
    Pimpl->Play(nullptr);
}
