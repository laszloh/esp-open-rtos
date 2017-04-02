/* i2s_audio_example.c - Plays wav file from spiffs.
 *
 * This example demonstrates how to use I2S with DMA to output audio.
 * The example is tested with TDA5143 16 bit DAC. But should work with
 * any I2S DAC.
 *
 * The example reads a file with name "sample.wav" from the file system and
 * feeds audio samples into DMA subsystem which outputs it into I2S bus.
 * Currently only 44100 Hz 16 bit 2 channel audio is supported.
 *
 * In order to test this example you need to place a file with name "sample.wav"
 * into directory "files". This file will be uploaded into spiffs on the device.
 * The size of the sample file must be less than 1MB to fit into spiffs image.
 * The format of the sample file must be 44100Hz, 16bit, 2 channels.
 * Also you need a DAC connected to ESP8266 to convert I2S stream to analog
 * output. Three wire must be connected: DATA, WS, CLOCK.
 *
 * This sample code is in the public domain.,
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "esp8266.h"

#include "fcntl.h"
#include "unistd.h"

#include <stdio.h>
#include <stdint.h>

#include "i2s_dma/i2s_dma.h"
#include "pcm514x/pcm514x.h"
#include "i2c/i2c.h"

#include "squeezelite/squeezelite.h"

#define DEMO	1
#define SERVER	"192.168.88.12:3483"
#define NAME	"espStreamer"

static const log_level level = lSDEBUG;

// PCM514x configuration
static const pcm514x_t config = {
	.mode = HWCFG_SPI,
	.cs = 0,
	.clk.out_en = false,
	.deemphasis = true
};

void dummy_task(void *pvParameters)
{
	unsigned rates[MAX_SUPPORTED_SAMPLERATES] = { 44100 };
	u8_t mac[6];

	sdk_wifi_get_macaddr(STATION_IF, mac);

    // init the DAC
    pcm514x_init(&config);

    // init the squeeze protocol engine
    stream_init(level, STREAMBUF_SIZE);
#ifdef DEMO
    output_init_stdout(level, OUTPUTBUF_SIZE, NULL, rates, 0);
#else
	#error "custom output not implemented yet"
#endif
    decode_init(level, NULL, "");
    slimproto(level, SERVER, mac, NAME, NULL, NULL, 0);

	while(1) {
		printf("Slimproto killed!\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void user_init(void)
{
    uart_set_baud(0, 115200);

    xTaskCreate(dummy_task, "dummy_task", 1024, NULL, 2, NULL);
}
