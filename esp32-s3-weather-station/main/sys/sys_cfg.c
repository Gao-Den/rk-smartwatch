/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   21/02/2025
 ******************************************************************************
**/

#include "sys_cfg.h"
#include "io_cfg.h"

static bool got_reset_reason = false;
static temperature_sensor_handle_t cpu_temp_handle = NULL;

/******************************************************************************
* system utility
*******************************************************************************/
void sys_ctrl_reset() {
    esp_restart();
}

void sys_ctrl_cpu_temperature_init() {
    temperature_sensor_config_t config = {
        .clk_src = TEMPERATURE_SENSOR_CLK_SRC_DEFAULT
    };

    temperature_sensor_install(&config, &cpu_temp_handle);
    temperature_sensor_enable(cpu_temp_handle);
}

float sys_ctrl_read_cpu_temperature() {
    float temp = 0;
    if (cpu_temp_handle != NULL) {
        temperature_sensor_get_celsius(cpu_temp_handle, &temp);
    }
    
    return temp;
}

void sys_ctrl_get_info(uint32_t* free_heap_size, uint32_t* minimum_free_heap_size, float* cpu_temperature) {
    size_t free_heap = esp_get_free_heap_size();
    size_t min_free = esp_get_minimum_free_heap_size();
    float cpu_temp = sys_ctrl_read_cpu_temperature();

    *free_heap_size = free_heap;
    *minimum_free_heap_size = min_free;
    *cpu_temperature = cpu_temp;
}

bool sys_ctrl_got_reset_reason() {
    return got_reset_reason;
}

const char* sys_ctrl_get_reset_reason() {

    got_reset_reason = true;

    switch (esp_reset_reason()) {
        case ESP_RST_UNKNOWN: {
            return "Unknown reset";
        }
            
        case ESP_RST_POWERON: {
            return "Power-on reset";
        }
            
        case ESP_RST_EXT: {
            return "External reset";
        }
            
        case ESP_RST_SW: {
            return "Software reset";
        }
            
        case ESP_RST_PANIC: {
            return "Panic reset";
        }
            
        case ESP_RST_INT_WDT: {
            return "Interrupt watchdog reset";
        }
            
        case ESP_RST_TASK_WDT: {
            return "Task watchdog reset";
        }
            
        case ESP_RST_WDT: {
            return "Other watchdog reset";
        }
            
        case ESP_RST_DEEPSLEEP: {
            return "Wakeup from deep sleep";
        }

        case ESP_RST_BROWNOUT: {
            return "Brownout reset";
        }

        case ESP_RST_SDIO: {
            return "Reset over SDIO";
        }
            
        default: {
            return "Invalid reset reason";
        }
    }
}

/******************************************************************************
* system watdog timer
*******************************************************************************/
void sys_ctrl_wdg_init(uint32_t timeout) {
    esp_task_wdt_deinit();

    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = timeout,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true
    };

    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);
}

void sys_ctrl_wdg_reset() {
    esp_task_wdt_reset();
}
