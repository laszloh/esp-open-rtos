# extrenal SPI SRAM driver for the ESP8266

This driver enables the access to an external SRAM buffer connected to the SPI bus. The buffer can be configured to meet the user needs. 

Following variables can be modified by the user:

- _Mode_: Supports FIFO and LIFO access
- _Size_: The Buffer can be configured to meet the user needs
- _Word size_: The length of a single word can be configured as needed
- _Number of buffers_: More than one buffer can be mapped to the SPI SRAM.


### Usage

````

#define ERAM_BUFFER_CNT	2				///< default buffer count: 1; When more than 1 are enabled, a semaphore will be used to prevent multiple access
#define ERAM_SRAM_SIZE	(1024*1024)		///< The maximum RAM size in _bytes_ 
#define ERAM_TEST_RAM	true			///< Run a RAM test at start

#include <extram/extram.h>

static const extram_t fifo = {
	.spi = {
		.id = 0,					///< The spi bus to use
		.cs_pin = 1,				///< gpio number of the chip select
	},
	.mode = ERAM_FIFO,				///< set up the buffer as an FIFO (supported: ERAM_FIFO, ERAM_LIFO)
	.size = 0,						///< 0 = use whole SRAM, >0 = number of words, must not be bigger than the avaliable space
	.wordsize = sizeof(uint32_t),	///< Size of a single element. @attention: All buffers must be multiples of this!
};


void func() {
	uint32_t buf[8];
	uint32_t x;
	uint32_t i;

	eram_init(&fifo);
	
	eram_write(0x01);
	eram_write_buf(&buf, 8);
	
	x = eram_read();
	eram_read_buf(&buf, 8);
	
	// fills the external ram
	while(eram_free() > =) {
		eram_write(0x01);
	}
	
	// and flushes everything down the drain :)
	eram_clear();
}


````

### License

The driver is released under the MIT license.

