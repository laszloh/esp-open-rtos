PROGRAM=esp-squeezelite

EXTRA_COMPONENTS = extras/i2s_dma extras/pcm514x extras/i2c

EXTRA_CFLAGS += -DSW_GAIN

PROGRAM_SRC_DIR=. ./squeezelite

# spiffs configuration
FLASH_SIZE = 32
#SPIFFS_BASE_ADDR = 0x80000
#SPIFFS_SIZE = 0x40000

include ../../../common.mk

$(eval $(call make_spiffs_image,files))

