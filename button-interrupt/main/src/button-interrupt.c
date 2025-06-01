#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"
#include "bitmaps.h"

#define TAG "SSD1306"
#define COLUMNS 128
#define ROWS 32
#define ROW_HEIGHT 8
#define ROW_LENGTH 32
#define BUTTON GPIO_NUM_5

void display_text_right_justified(const char *text, int page);
void display_bitmap_right_justified(const uint8_t *bitmap, int bitmap_dimension, int page);

TaskHandle_t h_displayTextTask = NULL;
QueueHandle_t interruptQueue;
SSD1306_t dev;
int lcd_width;

char line0[32];
char line1[32];
char line2[32];
char line3[32];

volatile char dealer[32];
volatile int team1_score;
volatile int team2_score;
volatile uint8_t suit[BITMAP_DIM];

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
	int pinNumber = (int)args;
	xQueueSendFromISR(interruptQueue, &pinNumber, NULL);
}

void update_team1_score_task(void *params)
{
	int pin_number = 0;
	uint32_t last_update = esp_timer_get_time() / 1000;
	uint32_t now;
	while (1)
	{
		if (xQueueReceive(interruptQueue, &pin_number, portMAX_DELAY))
		{
			now = esp_timer_get_time() / 1000;
			if (now - last_update > 300)
			{
				team1_score = team1_score + 1;
				last_update = now;
			}
		}
	}
}

void update_display_task(void *arg)
{
	while (1)
	{
		// Update text
		ssd1306_display_text(&dev, 0, line0, strlen(line0), false);
		ssd1306_display_text(&dev, 1, line1, strlen(line1), false);
		ssd1306_display_text(&dev, 2, line2, strlen(line2), false);
		ssd1306_display_text(&dev, 3, line3, strlen(line3), false);

		// Add dynamic text
		display_text_right_justified("Amelia", 0);
		display_bitmap_right_justified(DIAMOND, BITMAP_DIM, 1);
		
		char team1_score_str[2];
		sprintf(team1_score_str, "%d", team1_score);
		display_text_right_justified(team1_score_str, 2);
		display_text_right_justified("7", 3);

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

void app_main(void)
{
	ESP_LOGI(TAG, "INTERFACE is i2c");
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

	ssd1306_init(&dev, 128, 32);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_clear_screen(&dev, false);

	lcd_width = ssd1306_get_width(&dev);

	strcpy(line0, "Dealer:");
	strcpy(line1, "Trump:");
	strcpy(line2, "Team 1:");
	strcpy(line3, "Team 2:");

	team1_score = 0;

	esp_rom_gpio_pad_select_gpio(BUTTON);
	gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
	gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);
	gpio_set_intr_type(BUTTON, GPIO_INTR_NEGEDGE);

	interruptQueue = xQueueCreate(10, sizeof(int));
	xTaskCreate(update_team1_score_task, "update_team1_score", 2048, NULL, 1, NULL);

	gpio_install_isr_service(0);
	gpio_isr_handler_add(BUTTON, gpio_interrupt_handler, (void *)BUTTON);

	xTaskCreate(update_display_task, "update_display", 2048, NULL, 10, &h_displayTextTask);
}