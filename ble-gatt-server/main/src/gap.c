#include "gap.h"
#include "common.h"
#include "led.h"

// Private functions
inline static void format_addr(char *addr_str, uint8_t addr[]);
static void start_advertising(void);
static void print_conn_desc(struct ble_gap_conn_desc *desc);
static int gap_event_handler(struct ble_gap_event *event, void *arg);

// Private variables
static uint8_t own_addr_type;
static uint8_t addr_val[6] = {0};
static uint8_t esp_uri[] = {BLE_GAP_URI_PREFIX_HTTPS, '/', '/', 'e', 's', 'p', 'r', 'e', 's', 's', 'i', 'f', '.', 'c', 'o', 'm'};

inline static void format_addr(char *addr_str, uint8_t addr[]) {
    sprintf(addr_str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1],
            addr[2], addr[3], addr[4], addr[5]);
}

static void start_advertising(void) {
	// Initialize local variables
	int rc = 0;
	const char *name;
	struct ble_hs_adv_fields adv_fields = {0};	// advertising fields
	struct ble_hs_adv_fields rsp_fields = {0};	// response fields
	struct ble_gap_adv_params adv_params = {0};

	// Set advertisting flags
	adv_fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

	// Set device name in advertising fields
	name = ble_svc_gap_device_name();	// get the name we previously set in init
	adv_fields.name = (uint8_t *)name;
	adv_fields.name_len = strlen(name);
	adv_fields.name_is_complete = 1;

	// Set device transmit power
	adv_fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
	adv_fields.tx_pwr_lvl_is_present = 1;

	// Set device appearance
	adv_fields.appearance = BLE_GAP_APPEARANCE_GENERIC_TAG;
	adv_fields.appearance_is_present = 1;

	// Set device LE role to be a peripheral device
	adv_fields.le_role = BLE_GAP_LE_ROLE_PERIPHERAL;
	adv_fields.le_role_is_present = 1;

	// Apply advertising fields
	rc = ble_gap_adv_set_fields(&adv_fields);
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to set adv fields. Error code: %d", rc);
		return;
	}

	// Set device address for response
	rsp_fields.device_addr = addr_val;
	rsp_fields.device_addr_type = own_addr_type;
	rsp_fields.device_addr_is_present = 1;

	// Set advertising interval
	rsp_fields.adv_itvl = BLE_GAP_ADV_ITVL_MS(500);
	rsp_fields.adv_itvl_is_present = 1;

	// Set URI. I don't know what this is.
	rsp_fields.uri = esp_uri;
	rsp_fields.uri_len = sizeof(esp_uri);

	// Apply scan response fields
	rc = ble_gap_adv_rsp_set_fields(&rsp_fields);
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to set response scan fields. Error code: %d", rc);
		return;
	}

	// Set beacon to be connectable, and in general discoverable mode
	adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
	adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

	// Set advertising interval
	adv_params.itvl_max = BLE_GAP_ADV_ITVL_MS(510);
	adv_params.itvl_min = BLE_GAP_ADV_ITVL_MS(500);

	// Start advertising
	rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, gap_event_handler, NULL);
	
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to start advertising. Error code: %d", rc);
		return;
	}
	
	ESP_LOGI(TAG, "Advertising started!");
}

static int gap_event_handler(struct ble_gap_event *event, void *arg) {
	int rc = 0;
	struct ble_gap_conn_desc desc;

	switch(event->type) {
		case BLE_GAP_EVENT_CONNECT:
			// Log wether connection was successful
			ESP_LOGI(TAG, "Connection %s : Status=%d", event->connect.status == 0 ? "established" : "failed", event->connect.status);

			// Handle successful connection
			if (event->connect.status == 0) {
				rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
				if (rc != 0) {
					ESP_LOGE(TAG, "Failed to find connection via handle. Error code: %d", rc);
					return rc;
				}

				// Print connection descriptor and turn on LED
				print_conn_desc(&desc);
				led_on();

				// Update connection parameters
				struct ble_gap_upd_params params = {.itvl_min = desc.conn_itvl,
													.itvl_max = desc.conn_itvl,
													.latency = 3,
													.supervision_timeout = desc.supervision_timeout
													};
				rc = ble_gap_update_params(event->connect.conn_handle, &params);
				if (rc != 0) {
					ESP_LOGE(TAG, "Failed to update connection parameters. Error code: %d", rc);
					return rc;
				}
			} 
			// Handle failed connection
			else {
				// Reset advertising
				start_advertising();
			}
			return rc;
		
		case BLE_GAP_EVENT_DISCONNECT:
			ESP_LOGI(TAG, "Disconnected from peer. Reason: %d", event->disconnect.reason);

			led_off();

			// Restart advertising
			start_advertising();
			return rc;

		case BLE_GAP_EVENT_CONN_UPDATE:
			ESP_LOGI(TAG, "Connection updated! Status=%d", event->conn_update.status);

			// Print updated connection descriptor
			rc = ble_gap_conn_find(event->conn_update.conn_handle, &desc);
			if (rc != 0) {
				ESP_LOGE(TAG, "Failed to find connection by handle. Error code: %d", rc);
				return rc;
			}

			print_conn_desc(&desc);
			return rc;
	}
	return rc;
}

static void print_conn_desc(struct ble_gap_conn_desc *desc) {
	char addr_str[18] = {0};

	// Print connection handle
	ESP_LOGI(TAG, "Connection handle: %d", desc->conn_handle);

	// Print device id address
	format_addr(addr_str, desc->our_id_addr.val);
	ESP_LOGI(TAG, "Device id address: type=%d, value=%s", desc->our_id_addr.type, addr_str);

	// Print peer id address
	format_addr(addr_str, desc->peer_id_addr.val);
	ESP_LOGI(TAG, "Peer id address: type=%d, value=%s", desc->peer_id_addr.type, addr_str);

	// Print connection info
	ESP_LOGI(TAG,
			"conn_itvl=%d, conn_latency=%d, supervision_timeout=%d, "
			"encrypted=%d, authenticated=%d, bonded=%d\n",
			desc->conn_itvl, desc->conn_latency, desc->supervision_timeout,
			desc->sec_state.encrypted, desc->sec_state.authenticated,
			desc->sec_state.bonded);
}

int gap_init(void) {
	int rc = 0;

	// Initialize GAP service
	ble_svc_gap_init();

	// Set GAP device name
	rc = ble_svc_gap_device_name_set(DEVICE_NAME);
	if (rc != 0) {
		ESP_LOGE(TAG, "Faied to set device name to %s. Error code: %d", DEVICE_NAME, rc);
		return rc;
	}

	// Set GAP device appearance
	rc = ble_svc_gap_device_appearance_set(BLE_GAP_APPEARANCE_GENERIC_TAG);
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to set device appearance. Error code: %d", rc);
		return rc;
	}
	return rc;
}

void adv_init(void) {
	int rc = 0;
	char addr_str[18] = {0};

	// Make sure we have a proper BT identity address set
	rc = ble_hs_util_ensure_addr(0);
	if (rc != 0) {
		ESP_LOGE(TAG, "This device does not have any available BT addresses.");
		return;
	}

	// Get address to use while advertising
	rc = ble_hs_id_infer_auto(0, &own_addr_type);
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to infer address type. Error code: %d", rc);
		return;
	}

	// Copy device address
	rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
	if (rc != 0) {
		ESP_LOGE(TAG, "Failed to copy device address. Error code: %d", rc);
		return;
	}

	// Format address to be human readable and log
	format_addr(addr_str, addr_val);
	ESP_LOGI(TAG, "Device address: %s", addr_str);

	// Initialize LED
	led_init();

	start_advertising();
}