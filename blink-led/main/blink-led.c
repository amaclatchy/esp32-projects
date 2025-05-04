#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "esp_log.h"

# define EXTERN_LED GPIO_NUM_13
# define EXTERN_LED_DELAY 200

# define INTERN_LED GPIO_NUM_2
# define INTERN_LED_DELAY 1000

TaskHandle_t h_internLEDTask = NULL;
TaskHandle_t h_externLEDTask = NULL;

void extern_led_task(void *arg) {
	while(1) {
		gpio_set_level(EXTERN_LED, 1);
		vTaskDelay(EXTERN_LED_DELAY / portTICK_PERIOD_MS);
		gpio_set_level(EXTERN_LED, 0);
		vTaskDelay(EXTERN_LED_DELAY / portTICK_PERIOD_MS);
	}
}

void intern_led_task(void *arg) {
	while(1) {
		gpio_set_level(INTERN_LED, 1);
		vTaskDelay(INTERN_LED_DELAY / portTICK_PERIOD_MS);
		gpio_set_level(INTERN_LED, 0);
		vTaskDelay(INTERN_LED_DELAY / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
	gpio_reset_pin(INTERN_LED);
	gpio_set_direction(INTERN_LED, GPIO_MODE_OUTPUT);

	gpio_reset_pin(EXTERN_LED);
	gpio_set_direction(EXTERN_LED, GPIO_MODE_OUTPUT);

	xTaskCreate(extern_led_task, "externLED", 1000, NULL, 10, &h_externLEDTask);
	xTaskCreate(intern_led_task, "internLED", 1000, NULL, 10, &h_internLEDTask);
}