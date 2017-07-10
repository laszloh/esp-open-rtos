#include "espressif/esp_common.h"
#include <string.h>
#include "esp/uart.h"
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include <ssid_config.h>

#include <espressif/esp_sta.h>
#include <espressif/esp_wifi.h>

#include <paho_mqtt_c/MQTTESP8266.h>
#include <paho_mqtt_c/MQTTClient.h>

#include "errout.h"

/* You can use http://test.mosquitto.org/ to test mqtt_client instead
 * of setting up your own MQTT server */
#define MQTT_HOST ("volumio.home.lan")
#define MQTT_PORT 1883

#define MQTT_USER NULL
#define MQTT_PASS NULL

#define MQTT_TOPIC_BUTTON ("/haus/og/button/action")
#define MQTT_TOPIC_BATT ("/haus/og/button/batt")

#define PUB_MSG_LEN 16

static const char *msg_on = "ON";
static const char *msg_off = "OFF";

static void deep_sleep(void)
{
	sdk_system_deep_sleep(0);
	taskYIELD();
}

static void  mqtt_task(void *pvParameters)
{
    uint8_t status  = 0;
    uint8_t retries = 5;
    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    DEBUG("WiFi: connecting to WiFi\n\r");
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    while(1)
    {
		int ret = 0;
		struct mqtt_network network;
		mqtt_client_t client   = mqtt_client_default;
		mqtt_message_t message;
		mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;
		char mqtt_client_id[20] = "\0";
		uint8_t mqtt_buf[100];
		uint8_t mqtt_readbuf[100];
		char msg[PUB_MSG_LEN - 1] = "\0";
		
        while ((status != STATION_GOT_IP) && (retries)){
            status = sdk_wifi_station_get_connect_status();
            DEBUG("%s: status = %d\n\r", __func__, status );
            if( status == STATION_WRONG_PASSWORD ){
                DEBUG("WiFi: wrong password");
                break;
            } else if( status == STATION_NO_AP_FOUND ) {
                DEBUG("WiFi: AP not found");
                break;
            } else if( status == STATION_CONNECT_FAIL ) {
                DEBUG("WiFi: connection failed");
                break;
            }
            vTaskDelay( 1000 / portTICK_PERIOD_MS );
            --retries;
        }
        if (status != STATION_GOT_IP) {
            DEBUG("WiFi: connection failed");
			deep_sleep();
        }
		
		mqtt_network_new( &network );
		strcpy(mqtt_client_id, "ESP-");
		strcat(mqtt_client_id, get_my_id());

        DEBUG("%s: started", __func__);
        DEBUG("%s: (Re)connecting to MQTT server %s ... ",__func__, MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);
		if(ret) {
            DEBUG("error: %d", ret);
            deep_sleep();
        }
		DEBUG("done", ret);
        mqtt_client_new(&client, &network, 5000, mqtt_buf, 100, mqtt_readbuf, 100);

        data.willFlag = 0;
        data.MQTTVersion = 3;
        data.clientID.cstring = mqtt_client_id;
        data.username.cstring = MQTT_USER;
        data.password.cstring = MQTT_PASS;
        data.keepAliveInterval = 0;
        data.cleansession = 0;
        DEBUG("Send MQTT connect ... ");
        ret = mqtt_connect(&client, &data);
        if(ret){
            DEBUG("error: %d", ret);
            mqtt_network_disconnect(&network);
            deep_sleep();
        }
        DEBUG("done");

		message.payload = msg_on;
		message.payloadlen = PUB_MSG_LEN;
		message.dup = 0;
		message.qos = MQTT_QOS0;
		message.retained = 0;
		mqtt_publish(&client, MQTT_TOPIC_BUTTON, &message);
		message.payload = msg_off;
		message.payloadlen = PUB_MSG_LEN;
		mqtt_publish(&client, MQTT_TOPIC_BUTTON, &message);

		ret = mqtt_yield(&client, 1000);
		if (ret == MQTT_DISCONNECTED) {
			DEBUG("Connection dropped");
		} else {
			DEBUG("MQTT: success!");
		}
		mqtt_network_disconnect(&network);
		deep_sleep();
    }
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    DEBUG("SDK version:%s\n", sdk_system_get_sdk_version());

    xTaskCreate(&mqtt_task, "mqtt_task", 1024, NULL, 4, NULL);
}
