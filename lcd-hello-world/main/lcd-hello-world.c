#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"

#define TAG "SSD1306"
#define COLUMNS 128
#define ROWS 32

TaskHandle_t h_displayTextTask = NULL;
SSD1306_t dev;
char line0[32];
char line1[32];
char line2[32];
char line3[32];


void display_text_task(void *arg) {
	while(1) {
		ssd1306_display_text(&dev, 0, line0, strlen(line0), false);
		ssd1306_display_text(&dev, 1, line1, strlen(line1), false);
		ssd1306_display_text(&dev, 2, line2, strlen(line2), false);
		ssd1306_display_text(&dev, 3, line3, strlen(line3), false);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void app_main(void)
{
	ESP_LOGI(TAG, "INTERFACE is i2c");
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

	ssd1306_init(&dev, COLUMNS, ROWS);
	ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);

	strcpy(line0, "Dealer: Amelia"); 
	strcpy(line1, "Trump: Hearts"); 
	strcpy(line2, "Team 1: 3"); 
	strcpy(line3, "Team 2: 7"); 

	xTaskCreate(display_text_task, "textDisplay", 2048, NULL, 10, &h_displayTextTask);
}