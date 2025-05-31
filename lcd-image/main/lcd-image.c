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

// https://www.mischianti.org/2021/07/14/ssd1306-oled-display-draw-images-splash-and-animations-2/
uint8_t batman[] = {
		0b11111111, 0b11111111, 0b11111111, 0b11111111,
		0b11111111, 0b10011111, 0b11111001, 0b11111111,
		0b11111110, 0b00111110, 0b01111100, 0b01111111,
		0b11111000, 0b00111100, 0b00111100, 0b00011111,
		0b11110000, 0b00011100, 0b00111000, 0b00001111,
		0b11110000, 0b00000000, 0b00000000, 0b00001111,
		0b11100000, 0b00000000, 0b00000000, 0b00000111,
		0b11100000, 0b00000000, 0b00000000, 0b00000111,
		0b11110000, 0b00000000, 0b00000000, 0b00001111,
		0b11110000, 0b11000100, 0b00100011, 0b00001111,
		0b11111001, 0b11111110, 0b01111111, 0b10011111,
		0b11111100, 0b11111110, 0b01111111, 0b00111111,
		0b11111111, 0b11111111, 0b11111111, 0b11111111
};

SSD1306_t dev;

void app_main(void)
{
	ESP_LOGI(TAG, "INTERFACE is i2c");
	ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
	ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
	ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
	i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);


	ESP_LOGI(TAG, "Panel is 128x32");
	ssd1306_init(&dev, 128, 32);
	ssd1306_contrast(&dev, 0xff);
	ssd1306_clear_screen(&dev, false);

	while(1) {
	
		ssd1306_display_text(&dev, 0, "SSD1306 128x32", 14, false);
		ssd1306_display_text(&dev, 1, "BATMAN", 6, false);

		int bitmapWidth = 4*8;
		int width = ssd1306_get_width(&dev);
		int xpos = width / 2; // center of width
		xpos = xpos - bitmapWidth/2; 
		int ypos = 16;
		ESP_LOGD(TAG, "width=%d xpos=%d", width, xpos);
		ssd1306_bitmaps(&dev, xpos, ypos, batman, 32, 13, false);
		vTaskDelay(3000 / portTICK_PERIOD_MS);

		// for(int i=0;i<128;i++) {
		// 	ssd1306_wrap_arround(&dev, SCROLL_RIGHT, 2, 3, 0);
		// }
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}