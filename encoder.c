#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rotary_encoder.h"

static const char* tag = "encoder";

#define ROT_ENC_A_GPIO 34
#define ROT_ENC_B_GPIO 35
#define ENABLE_HALF_STEPS false  // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT          0      // Set to a positive non-zero number to reset the position if this value is exceeded
// #define FLIP_DIRECTION      // Set to true to reverse the clockwise/counterclockwise sense

rotary_encoder_info_t info = { 0 };
QueueHandle_t event_queue;

void (*on_encoder)(bool up);

void on_encoder_default(bool down)
{
    log_i("%s", down ? "down" : "up");
}

static void encoder_task(void* arg)
{
    for(;;)
    {
        rotary_encoder_event_t event = { 0 };
        if (xQueueReceive(event_queue, &event, portMAX_DELAY))
        {
            on_encoder(event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE);
            // ESP_LOGI(tag, "Event: position %d, direction %s", event.state.position,
            //          event.state.direction ? (event.state.direction == ROTARY_ENCODER_DIRECTION_CLOCKWISE ? "CW" : "CCW") : "NOT_SET");
        }    
    }
}

void encoder_init(void)
{
    on_encoder = on_encoder_default;
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif
    event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
    xTaskCreate(encoder_task, "encoder_task", 2048, NULL, 2, NULL);
}
