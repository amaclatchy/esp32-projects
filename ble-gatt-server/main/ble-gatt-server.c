#include "common.h"
#include "gap.h"

/* Library function declarations */
void ble_store_config_init(void);

/* Private function declarations */
static void on_stack_reset(int reason);
static void on_stack_sync(void);
static void nimble_host_config_init(void);
static void nimble_host_task(void *param);

static void nimble_host_config_init(void) {
	// Set host callbacks
	ble_hs_cfg.reset_cb = on_stack_reset;
	ble_hs_cfg.sync_cb = on_stack_sync;
	ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

	// Save the configuration
	ble_store_config_init();
}

static void on_stack_reset(int reason) {
	ESP_LOGI(TAG, "nimble stack reset, reset reason: %d", reason);
}

static void on_stack_sync(void) {
	adv_init();
}

static void nimble_host_task(void *param) {
	ESP_LOGI(TAG, "NimBLE host task has been started!");

	// Will hold program until nimble_port_stop() is executed
	nimble_port_run();

	// Clean up before exiting
	vTaskDelete(NULL);
}

void app_main(void)
{
	// Initialize non volatile storage
	esp_err_t ret = ESP_OK;
	ret = nvs_flash_init();
	
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		// If init failed because of space or versioning, erase it and try again
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to init nvs flash. Error code: %d", ret);
		return;
	}

	// Initialize NimBLE host stack
	ret = nimble_port_init();
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to init NimBLE stack. Error code: %d", ret);
		return;
	}

	// Initialize GAP service
	int rc = 0;
	rc = gap_init();
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to initialize GAP service. Error code: %d", rc);
		return;
	}

	// Configure NimBLE host
	nimble_host_config_init();

	// Start the NimBLE host task thread
	xTaskCreate(nimble_host_task, "amac NimBLE Host", 4*1024, NULL, 5, NULL);
	return;
}
