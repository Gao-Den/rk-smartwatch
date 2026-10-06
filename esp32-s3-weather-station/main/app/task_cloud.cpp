/**
 ******************************************************************************
 * @author: GaoDen
 * @date:   25/03/2025
 ******************************************************************************
**/

#include "task_cloud.h"

#include "lt_task.h"
#include "lt_message.h"
#include "lt_timer.h"
#include "lt_log.h"

#include "esp_mac.h"
#include "esp_wifi.h"

#include "io_cfg.h"
#include "sys_cfg.h"
#include "app.h"
#include "app_dbg.h"
#include "app_flash.h"
#include "app_network.h"
#include "task_list.h"

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "cJSON.h"

static gateway_config_t gw_inf;

/* weather polling */
static void weather_polling();

void task_cloud_handler(void* argv) {
    lt_msg_t* msg = (lt_msg_t*)0;
    waiting_active_object_ready();
    
    while (1) {

        msg = task_rev_msg(TASK_CLOUD_ID);

        switch (msg->signal) {
        case CLOUD_INIT: {
            APP_PRINT("[task_cloud] CLOUD_INIT\n");
            timer_set(TASK_CLOUD_ID, CLOUD_WEATHER_POLLING, 3000, TIMER_ONE_SHOT);
        }
            break;

        case CLOUD_WEATHER_POLLING: {
            APP_PRINT("[task_cloud] CLOUD_WEATHER_POLLING\n");
            weather_polling();
            timer_set(TASK_CLOUD_ID, CLOUD_WEATHER_POLLING, 15000, TIMER_ONE_SHOT);
        }   
            break;

        default: {
        }
            break;
        }

        /* free message */
        task_free_msg(msg);
    }
}

static esp_err_t http_get(const char *url, char *buffer, size_t buffer_size) {
    esp_http_client_config_t config = {};

    config.url = url;
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.timeout_ms = 10000;

    /* http client init */
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        return ESP_FAIL;
    }

    /* http set method */
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        return err;
    }

    /* http fetch headers */
    int content_length = esp_http_client_fetch_headers(client);
    APP_DBG("[http] content length: %d\n", content_length);

    /* http read response */
    int total_read = 0;
    while (total_read < buffer_size - 1) {
        int read_len = esp_http_client_read(client, buffer + total_read, buffer_size - 1 - total_read);
        if (read_len <= 0) {
            break;
        }

        total_read += read_len;
    }

    buffer[total_read] = '\0';

    /* http get status code */
    int status = esp_http_client_get_status_code(client);
    APP_DBG("[http] status code: %d\n", status);

    /* http cleanup */
    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    if (status != 200) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t weather_get_location() {
    char buffer[HTTP_BUFFER_SIZE];
    const char* url = "https://ipapi.co/json/";
    APP_DBG("[weather] getting ip location...\n");

    /* http get location */
    esp_err_t err = http_get(url, buffer, sizeof(buffer));
    if (err != ESP_OK) {
        APP_DBG("[weather] location http request failed\n");
        return err;
    }

    cJSON* root = cJSON_Parse(buffer);
    if (root == NULL) {
        APP_DBG("[weather] invalid location json\n");
        return ESP_FAIL;
    }

    /* parse location data */
    cJSON* city = cJSON_GetObjectItem(root, "city");
    cJSON* latitude  = cJSON_GetObjectItem(root, "latitude");
    cJSON* longitude = cJSON_GetObjectItem(root, "longitude");
    cJSON* timezone  = cJSON_GetObjectItem(root, "timezone");

    APP_DBG("[weather] location information:\n");

    if (city) {
        APP_DBG("city: %s\n", city->valuestring);
    }

    if (latitude) {
        APP_DBG("latitude: %.6f\n", latitude->valuedouble);
    }
    if (longitude) {
        APP_DBG("longitude: %.6f\n", longitude->valuedouble);
    }
    if (timezone) {
        APP_DBG("timezone: %s\n", timezone->valuestring);
    }

    /* free json object */
    cJSON_Delete(root);

    return ESP_OK;
}

esp_err_t weather_get(float latitude, float longitude) {
    static char buffer[HTTP_BUFFER_SIZE];
    static char url[512];

    snprintf(url, sizeof(url),
        "https://api.open-meteo.com/v1/forecast"
        "?latitude=%.6f"
        "&longitude=%.6f"
        "&current="
        "temperature_2m,"
        "relative_humidity_2m,"
        "apparent_temperature,"
        "precipitation_probability,"
        "precipitation,"
        "wind_speed_10m,"
        "weather_code"
        "&timezone=auto",
        latitude,
        longitude
    );

    APP_DBG("[weather] getting weather...\n");
    APP_DBG("[weather] url: %s", url);

    /* http get weather data */
    esp_err_t err = http_get(url, buffer, sizeof(buffer));
    if (err != ESP_OK) {
        APP_DBGE("[weather] weather http request failed\n");
        return err;
    }

    /* parse weather data */
    cJSON* root = cJSON_Parse(buffer);
    if (root == NULL) {
        APP_DBGE("[weather] invalid weather json\n");
        return ESP_FAIL;
    }

    /* parse current weather data */
    cJSON* current = cJSON_GetObjectItem(root, "current");
    if (current == NULL) {
        APP_DBGE("[weather] no current weather data\n");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    /* extract weather information */
    cJSON* temperature = cJSON_GetObjectItem(current, "temperature_2m");
    cJSON* humidity = cJSON_GetObjectItem(current, "relative_humidity_2m");
    cJSON* apparent = cJSON_GetObjectItem(current, "apparent_temperature");
    cJSON* precipitation_probability = cJSON_GetObjectItem(current, "precipitation_probability");
    cJSON* precipitation = cJSON_GetObjectItem(current, "precipitation");
    cJSON* wind = cJSON_GetObjectItem(current, "wind_speed_10m");
    cJSON* weather_code = cJSON_GetObjectItem(current, "weather_code");

    APP_DBG("[weather] current weather information:\n");
    if (temperature) {
        APP_DBG("[weather] temperature: %.1f C\n", temperature->valuedouble);
    }
    if (humidity) {
        APP_DBG("[weather] humidity: %.1f %%\n", humidity->valuedouble);
    }
    if (apparent) {
        APP_DBG("[weather] apparent temperature: %.1f C\n", apparent->valuedouble);
    }
    if (precipitation_probability) {
        APP_DBG("[weather] precipitation probability: %.1f %%\n", precipitation_probability->valuedouble);
    }
    if (precipitation) {
        APP_DBG("[weather] precipitation: %.1f mm\n", precipitation->valuedouble);
    }
    if (wind) {
        APP_DBG("[weather] wind speed: %.1f km/h\n", wind->valuedouble);
    }
    if (weather_code) {
        APP_DBG("[weather] weather code: %d\n", weather_code->valueint);
    }

    weather_broadcast_frame_t weather_data = {0};
    weather_data.temperature = (uint16_t)(temperature->valuedouble);
    weather_data.humidity = (uint16_t)(humidity->valuedouble);
    weather_data.wind_speed = (uint16_t)(wind->valuedouble);
    weather_data.precipitation_probability = (uint16_t)(precipitation_probability->valuedouble);
    weather_data.precipitation = (uint16_t)(precipitation->valuedouble);
    APP_DBG("[weather] weather data packed for broadcast:\n");
    APP_DBG("[weather] temperature: %d\n", weather_data.temperature);
    APP_DBG("[weather] humidity: %d\n", weather_data.humidity);
    APP_DBG("[weather] wind speed: %d\n", weather_data.wind_speed);
    APP_DBG("[weather] precipitation probability: %d\n", weather_data.precipitation_probability);
    APP_DBG("[weather] precipitation: %d\n", weather_data.precipitation);

    task_post_common_msg(TASK_IF_ID, IF_SEND_FRAME, (uint8_t*)&weather_data, sizeof(weather_broadcast_frame_t));

    /* free json object */
    cJSON_Delete(root);

    return ESP_OK;
}

void weather_polling() {
    static char buffer[HTTP_BUFFER_SIZE];
    const char *url = "https://ipwho.is/";

    /* http get location */
    if (http_get(url, buffer, sizeof(buffer)) != ESP_OK) {
        APP_DBGE("[weather] location http request failed\n");
        return;
    }

    cJSON* root = cJSON_Parse(buffer);
    if (root == NULL) {
        APP_DBGE("[weather] failed to parse location json\n");
        return;
    }

    /* extract latitude and longitude */
    cJSON* latitude = cJSON_GetObjectItem(root, "latitude");
    cJSON* longitude = cJSON_GetObjectItem(root, "longitude");

    /* weather get data */
    if (latitude && longitude) {
        APP_DBG("[weather] location: %.6f, %.6f\n", latitude->valuedouble, longitude->valuedouble);
        weather_get(latitude->valuedouble, longitude->valuedouble);
    }

    cJSON_Delete(root);
}
