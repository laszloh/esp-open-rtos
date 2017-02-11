# Component makefile for extras/pcm514x

# expected anyone using pcm514x driver includes it as 'pcm514x/pcm514x.h'
INC_DIRS += $(pcm514x_ROOT)..

# I2C support is of by default
PCM514X_I2C_SUPPORT ?= 0
# SPI support is on by default
PCM514X_SPI_SUPPORT ?= 1

# args for passing into compile rule generation
pcm514x_SRC_DIR = $(pcm514x_ROOT)

pcm514x_CFLAGS = -DPCM514X_SPI_SUPPORT=${PCM514X_SPI_SUPPORT} -DPCM514X_I2C_SUPPORT=${PCM514X_I2C_SUPPORT} ${CFLAGS}

$(eval $(call component_compile_rules,pcm514x))
