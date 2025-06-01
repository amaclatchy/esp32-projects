#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON GPIO_NUM_5
#define LED_PIN 2

int state = 0;
QueueHandle_t interputQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *args) {
	int pinNumber = (int)args;
	xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}

void led_control_task(void *params) {
	int pinNumber, count = 0;
	uint32_t last_change = esp_timer_get_time() / 1000;
	uint32_t now;
	while (1) {
		if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY)) {
			now = esp_timer_get_time() / 1000;
			if (now - last_change > 300) {
				printf("Diff is %lu, button state is %i\n", (now - last_change), gpio_get_level(LED_PIN));
				// printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BUTTON));
				int state = gpio_get_level(LED_PIN);
				gpio_set_level(LED_PIN, !state);
				last_change = now;
			}
		}
	}
}

void app_main() {
	esp_rom_gpio_pad_select_gpio(LED_PIN);
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

	esp_rom_gpio_pad_select_gpio(BUTTON);
	gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);
	gpio_set_intr_type(BUTTON, GPIO_INTR_NEGEDGE);

	interputQueue = xQueueCreate(10, sizeof(int));
	xTaskCreate(led_control_task, "led_control_task", 2048, NULL, 1, NULL);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON, gpio_interrupt_handler, (void *)BUTTON);
}