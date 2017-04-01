PROGRAM=i2s_pcm514x_example

EXTRA_COMPONENTS = extras/spiffs extras/i2s_dma extras/pcm514x extras/i2c

#EXTRA_CFLAGS+=-I../../../esp-gdbstub/include -DportREMOVE_STATIC_QUALIFIER -DINCLUDE_pcTaskGetTaskName=1 -Og -ggdb
#EXTRA_LDFLAGS+=-L../../../esp-gdbstub/lib

#CFLAGS+=-DportREMOVE_STATIC_QUALIFIER -DINCLUDE_pcTaskGetTaskName=1

# spiffs configuration
FLASH_SIZE = 8
SPIFFS_BASE_ADDR = 0x80000
SPIFFS_SIZE = 0x40000

include ../../common.mk

#LIBS+=esp-gdbstub
#PROGRAM_CFLAGS+=-O0

$(eval $(call make_spiffs_image,files))

