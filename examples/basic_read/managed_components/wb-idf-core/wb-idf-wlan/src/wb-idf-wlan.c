/**
 * @file wb-idf-wlan.c
 * @brief Implementation of Whirlingbits WLAN Driver for ESP-IDF.
 * @author Whirlingbits
 * @date 2024
 */

#include "wb-idf-wlan.h"

static const char *TAG = "WB-IDF-WLAN";

static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;


int wb_wifi_check_status()
{
	EventBits_t uxBits;
    int wifi_status = 0; 
	uxBits = xEventGroupGetBits(s_wifi_event_group);
	if(uxBits & WIFI_CONNECTED_BIT)
	{
		wifi_status = 1;
	}
	
	else if (!(uxBits & WIFI_CONNECTED_BIT))
	{
        wifi_status = 0;
	}
return wifi_status;
}

static void station_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WB_STATION_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }  else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_STOP){
        ESP_LOGI(TAG, "Wifi has been disconnected");
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void ap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wb_wifi_init_sta(char* sta_ssid, char* sta_password)
{
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &station_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &station_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    wifi_config_t wifi_config = {};
    	strcpy((char*)wifi_config.sta.ssid, sta_ssid);
        strcpy((char*)wifi_config.sta.password, sta_password);
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	     * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
        wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
        wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

void wb_wifi_init_ap(char* ap_ssid, char* ap_password)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &ap_event_handler,
                                                        NULL,
                                                        NULL));

        wifi_config_t wifi_config = {};
    	strcpy((char*)wifi_config.ap.ssid, ap_ssid);
    	wifi_config.ap.ssid_len = strlen(ap_ssid);
    	wifi_config.ap.channel = WB_AP_ESP_WIFI_CHANNEL;
    	strcpy((char*)wifi_config.ap.password,ap_password);
    	wifi_config.ap.max_connection = WB_AP_MAX_STA_CONN;

#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
        wifi_config.ap.authmode = WIFI_AUTH_WPA3_PSK;
        wifi_config.ap.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;        
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
        wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
#endif
        wifi_config.ap.pmf_cfg.required = true;

    if (strlen(ap_password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
            ap_ssid, ap_password, WB_AP_ESP_WIFI_CHANNEL);
}

// Function to start Station and softap coexistence mode
void wb_wifi_init_sta_softap(void){
    //WIFI_MODE_APSTA
}



void wb_wifi_deinit(void)
{
    ESP_LOGI(TAG, "Deinit wifi to change wifi mode without restart");
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_NULL);
}