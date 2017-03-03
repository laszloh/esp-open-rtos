/*
 * pcm514x.c
 *
 *  Created on: Dec 26, 2016
 *      Author: laszlo
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "pcm514x.h"
#include "pcm514x_regs.h"
#include <stdio.h>

#include <esp/gpio.h>
#include <esp/iomux_regs.h>

#if PCM514X_I2C_SUPPORT

#include <i2c/i2c.h>
#define I2C_ADDR(a)	(0x98 | (a & 0x07))

#endif

#if PCM514X_SPI_SUPPORT

#include <esp/spi.h>
#define SPI_BUS 1

#endif


#define CMD_SHIFT	1
#define CMD_BIT		0x01

#define cmd_read(r)		(((r)<<CMD_SHIFT) | CMD_BIT)
#define cmd_write(r)	(((r)<<CMD_SHIFT) | ~(CMD_BIT))

// debug levels
//   0..none
//   1..error
//   2..warning
//   3..debug

#ifndef PCM514X_DEBUG
#define PCM514X_DEBUG 0
#endif

#if PCM514X_DEBUG >= 1
#define debug(level, format, ...) if (PCM514X_DEBUG >= (level)) { printf("%s" format "\n", "pcm514x: ", ## __VA_ARGS__); }
#else
#define debug(level, format, ...)
#endif

static void write(const pcm514x_t* dev, uint8_t reg, uint8_t val);
static uint8_t read(const pcm514x_t* dev, uint8_t reg);

static int write_register(const pcm514x_t* dev, int reg, uint8_t mask, uint8_t val);
static int read_register(const pcm514x_t* dev, int reg, uint8_t mask, uint8_t shift, uint8_t *val);

pcm514x_error_t pcm514x_init(const pcm514x_t* dev)
{
	int ret;

	// setup the communication interface
	switch(dev->mode) {
#if (PCM514X_SPI_SUPPORT)
	case HWCFG_SPI:
		// set up the interface
		gpio_enable(dev->cs, GPIO_OUTPUT);
		gpio_write(dev->cs, true);
		spi_init(SPI_BUS, SPI_MODE0, SPI_FREQ_DIV_8M, true, SPI_LITTLE_ENDIAN, true);
		break;
#endif

	case HWCFG_HW:
		debug(3, "Hardwired mode is not fully supported!");
		if(dev->hw.demp >= 0)
			gpio_enable(dev->hw.demp, GPIO_OUTPUT);
		if (dev->hw.att0 >= 0 && dev->hw.att1 >= 0 && dev->hw.att2 >= 0) {
			gpio_enable(dev->hw.att0, GPIO_OUTPUT);
			gpio_enable(dev->hw.att1, GPIO_OUTPUT);
			gpio_enable(dev->hw.att2, GPIO_OUTPUT);
		}
		if(dev->hw.agns >= 0)
			gpio_enable(dev->hw.agns, GPIO_OUTPUT);
		if(dev->hw.flt >= 0)
			gpio_enable(dev->hw.flt, GPIO_OUTPUT);
		return PCM514x_ERR_OK;

#if (PCM514X_I2C_SUPPORT)
	case HWCFG_I2C:
		// we do not have to do anything here, caller is responsible to set up I2C
		break;
#endif

	default:
		// unknown mode asked
		debug(1, "Unknown or unsupported mode: %d", dev->mode);
		return PCM514x_ERR_HWCFG_UNSUPPORTED;
	}

	// reset the PCM514x
	ret = write_register(dev, PCM514x_RESET, (PCM514x_RSTR | PCM514x_RSTM),
			(PCM514x_RSTR | PCM514x_RSTM));
	if(ret != PCM514x_ERR_OK)
		return ret;

	// setup the clk sources
	if(dev->clk.out_en) {
		// clk out from the esp is currently not supported
		debug(1, "SCK generation through PLL and external clock is not implemented!");
		return PCM514x_ERR_HWCFG_UNSUPPORTED;

	} else {
		// setup BCLK as clk source

		// ignore clk error
		ret = write_register(dev, PCM514x_ERROR_DETECT,
				PCM514x_IDCH, PCM514x_IDCH);
		if(ret != PCM514x_ERR_OK) {
			debug(1, "Failed to set ignore bits in register PCM514x_ERROR_DETECT: %d", ret);
			return ret;
		}

		// set the PLL to the BCLK
		ret = write_register(dev, PCM514x_PLL_REF,
				PCM514x_SREF, PCM514x_SREF);
		if(ret != PCM514x_ERR_OK) {
			debug(1, "Failed to switch PLL to BCLK: %d", ret);
			return ret;
		}
	}

	// default to standby mode
	ret = write_register(dev,  PCM514x_POWER,
			PCM514x_RQST, PCM514x_RQST);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to request standby: %d", ret);
		return ret;
	}

	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_set_standby(const pcm514x_t* dev, bool enable)
{
	uint8_t value = (enable) ? PCM514x_RQST : 0;
	int ret;

	// change the state of the device
	ret = write_register(dev, PCM514x_POWER, PCM514x_RQST, value);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to set standby to %d: %d", enable, ret);
		return ret;
	}
	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_get_standby(const pcm514x_t* dev, bool* enable)
{
	int ret;
	uint8_t val;

	ret = read_register(dev, PCM514x_POWER,	PCM514x_RQST,
			PCM514x_RQST_SHIFT, &val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to get standby state: %d", ret);
		return ret;
	}
	*enable = (val & PCM514x_RQST);
	return PCM514x_ERR_OK;
}


pcm514x_error_t pcm514x_set_volume(const pcm514x_t* dev, uint8_t vol)
{
	int ret;

	// write the right channel (remember, 0x00 = +24 dB, 0xFF =-102 dB)
	ret = write_register(dev, PCM514x_DIGITAL_VOLUME_2, 0xFF, ~vol);
	if(ret != PCM514x_ERR_OK){
		debug(1, "Failed to set left channel volume: %d", ret);
		return ret;
	}
	return write_register(dev, PCM514x_DIGITAL_VOLUME_3, 0xFF, ~vol);
	if(ret != PCM514x_ERR_OK){
		debug(1, "Failed to set right channel volume: %d", ret);
		return ret;
	}
	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_get_volume(const pcm514x_t* dev, uint8_t* vol)
{
	int ret;
	uint8_t val;

	ret = read_register(dev, PCM514x_DIGITAL_VOLUME_2, 0xFF, 0, &val);
	if(ret != PCM514x_ERR_OK){
		debug(1, "Failed to get audio volume: %d", ret);
		return ret;
	}
	*vol = ~val;
	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_set_volume_ramp(const pcm514x_t* dev, pcm514x_rampsrc_t src, pcm514x_volramp_t ramp)
{
	uint8_t reg = PCM514x_DIGITAL_MUTE_1;
	uint8_t val = 0x00;
	uint8_t mask = 0x00;
	int ret;

	switch(src) {
	case VOL_RAMP_UP:
		val = (ramp.freq << PCM514x_VNUF_SHIFT | ramp.step << PCM514x_VNUS_SHIFT);
		mask = (PCM514x_VNUF | PCM514x_VNUS);
		break;
	case VOL_RAMP_DOWN:
		val = (ramp.freq << PCM514x_VNDF_SHIFT | ramp.step << PCM514x_VNDS_SHIFT);
		mask = (PCM514x_VNDF | PCM514x_VNDS);
		break;
	case VOL_RAMP_EMERGENCY:
		reg = PCM514x_DIGITAL_MUTE_2;
		val = (ramp.freq << PCM514x_VEDF_SHIFT | ramp.step << PCM514x_VEDS_SHIFT);
		mask = (PCM514x_VEDF | PCM514x_VEDS);
		break;
	default:
		debug(1, "unknown ramp %d!", src);
		return PCM514x_ERR_OOB;
	}

	ret = write_register(dev, reg, mask, val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "failed to write ramp register %d: %d", reg, ret);
	}
	return ret;
}

pcm514x_error_t pcm514x_get_volume_ramp(const pcm514x_t* dev, pcm514x_rampsrc_t src, pcm514x_volramp_t* ramp)
{
	uint8_t reg = PCM514x_DIGITAL_MUTE_1;
	uint8_t val;
	int ret;

	if(src == VOL_RAMP_EMERGENCY)
		reg = PCM514x_DIGITAL_MUTE_2;

	ret = read_register(dev, reg, 0xFF, 0, & val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "failed to read ramp register %d: %d", reg, ret);
		return ret;
	}

	switch(src) {
	case VOL_RAMP_UP:
		ramp->freq = (val & PCM514x_VNUF) >> PCM514x_VNUF_SHIFT;
		ramp->step = (val & PCM514x_VNUS) >> PCM514x_VNUS_SHIFT;
		break;
	case VOL_RAMP_DOWN:
		ramp->freq = (val & PCM514x_VNDF) >> PCM514x_VNDF_SHIFT;
		ramp->step = (val & PCM514x_VNDS) >> PCM514x_VNDS_SHIFT;
		break;
	case VOL_RAMP_EMERGENCY:
		ramp->freq = (val & PCM514x_VEDF) >> PCM514x_VEDF_SHIFT;
		ramp->step = (val & PCM514x_VEDS) >> PCM514x_VEDS_SHIFT;
		break;
	default:
		debug(1, "unknown ramp %d!", src);
		return PCM514x_ERR_OOB;
	}
	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_set_audio_amute(const pcm514x_t* dev, pcm514x_audio_amute_t amute_r, pcm514x_audio_amute_t amute_l)
{
	uint8_t val;
	int ret;

	// setup auto mute
	val = ((amute_r.en_amute) ? 1 : 0)  << PCM514x_AMRE_SHIFT | ((amute_l.en_amute) ? 1 : 0) << PCM514x_AMLE_SHIFT |
			((amute_r.en_amute == amute_l.en_amute) ? 1 : 0) << PCM514x_ACTL_SHIFT;
	ret = write_register(dev, PCM514x_DIGITAL_MUTE_3, (PCM514x_ACTL | PCM514x_AMLE | PCM514x_AMRE), val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to write register 0x%02x to 0x%02x: %d", PCM514x_DIGITAL_MUTE_3, val, ret);
		return ret;
	}
	val = (amute_r.amute << PCM514x_ATMR_SHIFT) | (amute_l.amute << PCM514x_ATML_SHIFT);
	ret = write_register(dev, PCM514x_AUTO_MUTE, (PCM514x_ATMR | PCM514x_ATML), val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to write register 0x%02x to 0x%02x: %d", PCM514x_AUTO_MUTE, val, ret);
		return ret;
	}
	return PCM514x_ERR_OK;
}

pcm514x_error_t pcm514x_set_audio_dpath(const pcm514x_t* dev, pcm514x_audi_dpath_t dpath_r, pcm514x_audi_dpath_t dpath_l)
{
	uint8_t val;
	int ret;

	if(dpath_r >= DPATH_MAX || dpath_l >= DPATH_MAX){
		debug(1, "Invalid values dpath_r: 0x%02x dpath_l: 0x%02x", dpath_r, dpath_l);
		return PCM514x_ERR_OOB;
	}

	// set up data path routing
	val = (dpath_r << PCM514x_AUPR_SHIFT) | (dpath_r << PCM514x_AUPL_SHIFT);
	ret = write_register(dev, PCM514x_DAC_ROUTING, (PCM514x_AUPR | PCM514x_AUPL), val);
	if(ret != PCM514x_ERR_OK) {
		debug(1, "Failed to write register 0x%02x to 0x%02x: %d", PCM514x_DAC_ROUTING, val, ret);
		return ret;
	}
	return PCM514x_ERR_OK;
}

static int write_register(const pcm514x_t* dev, int reg, uint8_t mask, uint8_t val)
{
	uint8_t value;

	// change to the needed page
	write(dev, PCM514x_PAGE_REG, PCM514x_PAGE(reg));

	// read the current register value
	value = read(dev, reg);
	// replace the masked parts
	value = (val & mask) | (value & ~mask);
	write(dev, reg & 0xFF, value);

	// write failed
	if (value != read(dev, reg)){
		debug(1, "SPI write at 0x%02x failed, got 0x%02x (expected: 0x%02x)!", reg, value, val);
		return PCM514x_ERR_IOERROR;
	}
	return PCM514x_ERR_OK;
}


static int read_register(const pcm514x_t* dev, int reg, uint8_t mask, uint8_t shift, uint8_t *val)
{
	uint8_t value;

	// change to the needed page
	write(dev, PCM514x_PAGE_REG, PCM514x_PAGE(reg));

	// read the current register value
	value = read(dev, reg);
	value = (value & mask) >> shift;
	*val = value;
	return PCM514x_ERR_OK;
}


static void write(const pcm514x_t* dev, uint8_t reg, uint8_t val)
{
	debug(3, "write: reg: 0x%02x val: 0x%02x", reg, val);

	switch(dev->mode) {
#if PCM514X_SPI_SUPPORT
	case HWCFG_SPI:
		gpio_write(dev->cs, false);
		// shift register and clear r/!w bit (0x01)
		spi_transfer_8(SPI_BUS, cmd_write(reg));
		spi_transfer_8(SPI_BUS, val);
		gpio_write(dev->cs, true);
		break;
#endif
#if PCM514X_I2C_SUPPORT
	case HWCFG_I2C:
		i2c_start();
		if(!i2c_write(cmd_write(I2C_ADDR(dev->addr)))) {
			debug(1, "Error while transmitting salve address!");
			i2c_stop();
			return;
		}
		if(!i2c_write(reg)) {
			debug(1, "Error while transmitting register address!");
			i2c_stop();
			return;
		}
		if(!i2c_write(val)) {
			debug(1, "Error while transmitting value!");
		}
		i2c_stop();
		break;
#endif
	case HWCFG_HW:
		switch(reg) {
		default:
			break;
		}
		break;
	default:
		debug(1, "Unknown or unsupported mode: %d", dev->mode);
		break;
	}
}

static uint8_t read(const pcm514x_t* dev, uint8_t reg)
{
	uint8_t val = 0;

	debug(3, "read: reg: 0x%02x val: 0x%02x", reg, val);
	switch(dev->mode) {
#if PCM514X_SPI_SUPPORT
	case HWCFG_SPI:
		gpio_write(dev->cs, false);
		// shift register and clear r/!w bit (0x01)
		spi_transfer_8(SPI_BUS, cmd_read(reg));
		val = spi_transfer_8(SPI_BUS, 0x00);
		gpio_write(dev->cs, true);
		break;
#endif
#if PCM514X_I2C_SUPPORT
	case HWCFG_I2C:
		i2c_start();
		if(!i2c_write(cmd_write(I2C_ADDR(dev->addr)))) {
			debug(1, "Error while transmitting salve address!");
			i2c_stop();
			return 0;
		}
		if(!i2c_write(reg)) {
			debug(1, "Error while transmitting register address!");
			i2c_stop();
			return 0;
		}
		i2c_start();
		if(!i2c_write(cmd_read(I2C_ADDR(dev->addr)))) {
			debug(1, "Error while transmitting salve address after restart!");
			i2c_stop();
			return 0;
		}
		val = i2c_read(true);
		i2c_stop();
		break;
#endif
	case HWCFG_HW:
		switch(reg) {
		}
		break;
	default:
		debug(1, "Unknown or unsupported mode: %d", dev->mode);
		break;
	}

	return val;
}

