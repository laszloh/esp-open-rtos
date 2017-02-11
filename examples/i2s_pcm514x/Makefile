PROGRAM=i2s_pcm514x_example
EXTRA_COMPONENTS = extras/spiffs extras/i2s_dma extras/pcm514x extras/i2c

# spiffs configuration
FLASH_SIZE = 8
SPIFFS_BASE_ADDR = 0x80000
SPIFFS_SIZE = 0x40000

include ../../common.mk

$(eval $(call make_spiffs_image,files))
