#ifndef LED_H
#define LED_H

/* Includes */
#include "driver/gpio.h"
#include "sdkconfig.h"

/* Defines */
#define INTERN_LED GPIO_NUM_2

/* Public function declarations */
uint8_t get_led_state(void);
void led_on(void);
void led_off(void);
void led_init(void);

#endif // LED_H