/*
 * pcm514x.h
 *
 *  Created on: Dec 26, 2016
 *      Author: laszlo
 */

#ifndef ESP_FIRMWARE_PCM514X_PCM514X_H_
#define ESP_FIRMWARE_PCM514X_PCM514X_H_

#include <stdint.h>
#include <stdbool.h>

#include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * I/O protocols
 */
typedef enum {
	HWCFG_HW = 0,
	HWCFG_SPI,
	HWCFG_I2C,
	HWCFG_MAX
} pcm514x_protocol_t;

typedef struct{
	pcm514x_protocol_t mode;		///< HW communication protocol (currently only SPI supported)
	union {
		// pins of the esp
		struct {
			int8_t demp;		///< De-emphasis control for 44.1-kHz sampling rate: Off (Low) / On (High)
			int8_t att0;		///< Attenuation, Bit 0
			int8_t att1;		///< Attenuation, Bit 1
			int8_t att2;		///< Attenuation, Bit 2
			int8_t agns;		///< Analog gain selector: 0-dB 2-V RMS output (Low), 6-dB 1-V RMS output (High)
			int8_t flt;			///< Filter select: Normal latency (Low) / Low latency (High)
		} hw;
#if PCM514X_SPI_SUPPORT
		uint8_t cs;				///< SW chip select pin on the esp
#endif
#if PCM514X_I2C_SUPPORT
		uint8_t addr;			///< I2C address of the pcm514x
#endif
	};
	struct {
		bool out_en;			///< Clock output from the esp: None (false) / XTAL (true)
		uint32_t speed;			///< The clock speed in Hz (should be 24000000 or 26000000 depending on the crystal)
		int8_t pll_in;			///< GPIO pin on the pcm for the PLL
		int8_t pll_out;			///< GPIO pin on the pcm for the output of the pll (into sclk)
	} clk;
	bool deemphasis;			///< Enable the 44.1 KHz de-emphasis module
} pcm514x_t;

typedef enum {
	PCM514x_ERR_OK = 0,						///< Command executed without error
	PCM514x_ERR_HWCFG_UNSUPPORTED = -1,		///< Unsupported HW configuration given
	PCM514x_ERR_PCM_UNSUPPORTED = -2,		///< Unsupported audio configuration given
	PCM514x_ERR_NOMEM = -3,					///< Out of memory
	PCM514x_ERR_IOERROR = -4,				///< Register write failed
	PCM514x_ERR_OOB = -5,					///< A supplied value was out of bound
} pcm514x_error_t;

typedef enum {
	AUTOMUTE_21MS = 0,
	AUTOMUTE_106MS,
	AUTOMUTE_213MS,
	AUTOMUTE_533MS,
	AUTOMUTE_1070MS,
	AUTOMUTE_2130MS,
	AUTOMUTE_5330MS,
	AUTOMUTE_10660MS,
	AUTOMUTE_MAX
} pcm514x_automute_t;

typedef struct {
	bool en_amute;
	pcm514x_automute_t amute;
} pcm514x_audio_amute_t;

typedef enum {
	DPATH_MUTE = 0,
	DPATH_LEFT,
	DPATH_RIGHT,
	DPATH_MAX
} pcm514x_audi_dpath_t;

typedef struct {
	enum {
		FREQ_FS_1 = 0,
		FREQ_FS_2,
		FREQ_FS_4,
		FREQ_INSTANT,
	} freq;
	enum {
		STEP_4DB = 0,
		STEP_2DB,
		STEP_1DB,
		STEP_0_5DB,
	}step;
} pcm514x_volramp_t;

typedef enum {
	VOL_RAMP_UP = 0,
	VOL_RAMP_DOWN,
	VOL_RAMP_EMERGENCY,
	VOL_RAMP_MAX
} pcm514x_rampsrc_t;

typedef enum {
	GPIO_OFF = 0,
	GPIO_DSP_OUT,
	GPIO_REG,
	GPIO_FLAG_AMUTE,
	GPIO_FLAG_AMUTE_L,
	GPIO_FLAG_AMUTE_R,
	GPIO_CLK_INV,
	GPIO_SDOUT,
	GPIO_AMUTE_L,
	GPIO_AMUTE_R,
	GPIO_FLAG_PLL_LOCK,
	GPIO_FLAG_CHRG_PUMP,
	GPIO_RES1,
	GPIO_RES2,
	GPIO_FLAG_UV_07V,
	GPIO_FLAG_UV_03V,
	GPIO_PLL_DIV_OUT
} pcm514x_gpio_t;

pcm514x_error_t pcm514x_init(const pcm514x_t* dev);

pcm514x_error_t pcm514x_set_standby(const pcm514x_t* dev, bool enable);
pcm514x_error_t pcm514x_get_standby(const pcm514x_t* dev, bool* enable);

pcm514x_error_t pcm514x_set_volume(const pcm514x_t* dev, uint8_t vol);
pcm514x_error_t pcm514x_get_volume(const pcm514x_t* dev, uint8_t* vol);

pcm514x_error_t pcm514x_set_volume_ramp(const pcm514x_t* dev, pcm514x_rampsrc_t src, pcm514x_volramp_t ramp);
pcm514x_error_t pcm514x_get_volume_ramp(const pcm514x_t* dev, pcm514x_rampsrc_t src, pcm514x_volramp_t* ramp);

pcm514x_error_t pcm514x_set_audio_amute(const pcm514x_t* dev, pcm514x_audio_amute_t amute_r, pcm514x_audio_amute_t amute_l);
pcm514x_error_t pcm514x_get_audio_amute(const pcm514x_t* dev, pcm514x_audio_amute_t *amute_r, pcm514x_audio_amute_t *amute_l);

pcm514x_error_t pcm514x_set_audio_dpath(const pcm514x_t* dev, pcm514x_audi_dpath_t dpath_r, pcm514x_audi_dpath_t dpath_l);
pcm514x_error_t pcm514x_get_audio_dpath(const pcm514x_t* dev, pcm514x_audi_dpath_t *dpath_r, pcm514x_audi_dpath_t *dpath_l);

// gpio submodule

pcm514x_error_t pcm514x_gpio_enable(const pcm514x_t* dev, uint8_t gpio, bool output, bool invert);
pcm514x_error_t pcm514x_gpio_set_mode(const pcm514x_t* dev, uint8_t gpio, pcm514x_gpio_t mode);

pcm514x_error_t pcm514x_gpio_write(const pcm514x_t* dev, uint8_t gpio, bool output);

#ifdef __cplusplus
extern "C"
}
#endif

#endif /* ESP_FIRMWARE_PCM514X_PCM514X_H_ */
