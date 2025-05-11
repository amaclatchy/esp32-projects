#include "led.h"
#include "common.h"

// Private variables
static uint8_t led_state;

uint8_t get_led_state(void) { return led_state; }

void led_init(void) {
	gpio_reset_pin(INTERN_LED);
	gpio_set_direction(INTERN_LED, GPIO_MODE_OUTPUT);
	ESP_LOGI(TAG, "Internal LED configured!");
}

void led_on(void) { gpio_set_level(INTERN_LED, 1); }

void led_off(void) { gpio_set_level(INTERN_LED, 0); }