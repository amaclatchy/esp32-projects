#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "iot_button.h"
#include "button_gpio.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "bitmaps.h"

#define TAG "SSD1306"
#define COLUMNS 128
#define ROWS 32
#define ROW_HEIGHT 8
#define ROW_LENGTH 32
#define DEALER_BUTTON GPIO_NUM_15
#define SUIT_BUTTON GPIO_NUM_18
#define TEAM1_BUTTON GPIO_NUM_4
#define TEAM2_BUTTON GPIO_NUM_5
#define BUTTON_ACTIVE_LEVEL 0

void display_text_right_justified(const char *text, int page);
void display_bitmap_right_justified(const uint8_t *bitmap, int bitmap_dimension, int page);
void setup_button(gpio_num_t pin, button_cb_t f_single_callback, button_cb_t f_double_callback);
void increment_dealer(void *arg, void *usr_data);
void decrement_dealer(void *arg, void *usr_data);
void increment_suit(void *arg, void *usr_data);
void increment_team1(void *arg, void *usr_data);
void decrement_team1(void *arg, void *usr_data);
void increment_team2(void *arg, void *usr_data);
void decrement_team2(void *arg, void *usr_data);

TaskHandle_t h_displayTextTask = NULL;
QueueHandle_t interruptQueue;
SSD1306_t dev;
int lcd_width;
const char *players[4];
const uint8_t *suits[4];

char line0[32];
char line1[32];
char line2[32];
char line3[32];

volatile int current_dealer;
volatile int current_suit;
volatile int team1_score;
volatile int team2_score;
volatile uint8_t suit[BITMAP_DIM];

void increment_dealer(void *arg, void *usr_data)
{
	if (current_dealer >= 3) {
		current_dealer = 0;
	} else {
		current_dealer = current_dealer + 1;
	}
	current_suit = -1;
}

void decrement_dealer(void *arg, void *usr_data)
{
	if (current_dealer <= 0) {
		current_dealer = 3;
	} else {
		current_dealer = current_dealer - 1;
	}
	current_suit = -1;
}

void increment_suit(void *arg, void *usr_data)
{
	if (current_suit >= 3 || current_suit < 0) {
		current_suit = 0;
	} else {
		current_suit = current_suit + 1;
	}
}

void increment_team1(void *arg, void *usr_data)
{
	team1_score = team1_score + 1;
}

void decrement_team1(void *arg, void *usr_data)
{
	if (team1_score > 0) {
		team1_score = team1_score - 1;
	}
}

void increment_team2(void *arg, void *usr_data)
{
	team2_score = team2_score + 1;
}

void decrement_team2(void *arg, void *usr_data)
{
	if (team2_score > 0) {
		team2_score = team2_score - 1;
	}
}

void update_display_task(void *arg)
{
	int displayed_dealer = -1;
	int displayed_suit = -1;
	while (1)
	{
		if(displayed_dealer != current_dealer) {
			int cLen = (lcd_width/8) - (strlen(line0));
			char *str = malloc(cLen + 1);
			memset(str, ' ', cLen);
			str[cLen] = '\0';
			display_text_right_justified(str, 0);
			displayed_dealer = current_dealer;
			display_text_right_justified(players[current_dealer], 0);
		}

		if (current_suit < 0) {
			display_bitmap_right_justified(BLANK, BITMAP_DIM, 1);
			displayed_suit = -1;
		}
		else if (displayed_suit != current_suit) {
			display_bitmap_right_justified(suits[current_suit], BITMAP_DIM, 1);
			displayed_suit = current_suit;
		}
		
		char team1_score_str[2];
		sprintf(team1_score_str, "%d", team1_score);
		display_text_right_justified(team1_score_str, 2);
		char team2_score_str[2];
		sprintf(team2_score_str, "%d", team2_score);
		display_text_right_justified(team2_score_str, 3);

		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void display_bitmap_right_justified(const uint8_t *bitmap, int bitmap_dimension, int page)
{
	int x_pos = lcd_width - bitmap_dimension;
	int y_pos = page * ROW_HEIGHT;
	ssd1306_bitmaps(&dev, x_pos, y_pos, bitmap, bitmap_dimension, bitmap_dimension, false);
}

void display_text_right_justified(const char *text, int page)
{
	int _text_len = strlen(text);
	if (_text_len > 16)
		_text_len = 16;

	uint8_t image[8];
	int starting_x = lcd_width - (8 * _text_len);
	int x_pos = starting_x;
	for (int i = 0; i < _text_len; i++)
	{
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		ssd1306_display_image(&dev, page, x_pos, image, 8);
		x_pos = x_pos + 8;
	}
}

void setup_button(gpio_num_t pin, button_cb_t f_single_callback, button_cb_t f_double_callback)
{
	gpio_reset_pin(pin);
	gpio_set_direction(pin, GPIO_MODE_INPUT);
	gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);

	button_config_t button_config = {
		.long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
		.short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS};
	button_gpio_config_t button_gpio_config = {
		.active_level = BUTTON_ACTIVE_LEVEL,
		.gpio_num = pin,
	};
	button_handle_t gpio_btn = NULL;

	iot_button_new_gpio_device(&button_config, &button_gpio_config, &gpio_btn);
	if (NULL == gpio_btn)
	{
		ESP_LOGE(TAG, "Button create failed");
	}
	if (f_single_callback != NULL) {
		iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, NULL, f_single_callback, NULL);
	}
	if (f_double_callback != NULL) {
		iot_button_register_cb(gpio_btn, BUTTON_DOUBLE_CLICK, NULL, f_double_callback, NULL);
	}
}

void app_main(void)
{
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
	ssd1306_init(&dev, 128, 32);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_clear_screen(&dev, false);

	strcpy(line0, "Dealer:");
	strcpy(line1, "Trump:");
	strcpy(line2, "Team 1:");
	strcpy(line3, "Team 2:");
	ssd1306_display_text(&dev, 0, line0, strlen(line0), false);
	ssd1306_display_text(&dev, 1, line1, strlen(line1), false);
	ssd1306_display_text(&dev, 2, line2, strlen(line2), false);
	ssd1306_display_text(&dev, 3, line3, strlen(line3), false);

	lcd_width = ssd1306_get_width(&dev);

	players[0] = "Amelia";
	players[1] = "Mark";
	players[2] = "Fay";
	players[3] = "Tyler";

	suits[0] = HEART;
	suits[1] = DIAMOND;
	suits[2] = CLUB;
	suits[3] = SPADE;

	team1_score = 0;
	team2_score = 0;
	current_dealer = 0;
	current_suit = -1;

	setup_button(DEALER_BUTTON, increment_dealer, decrement_dealer);
	setup_button(SUIT_BUTTON, increment_suit, NULL);
	setup_button(TEAM1_BUTTON, increment_team1, decrement_team1);
	setup_button(TEAM2_BUTTON, increment_team2, decrement_team2);

	xTaskCreate(update_display_task, "update_display", 2048, NULL, 10, &h_displayTextTask);
}