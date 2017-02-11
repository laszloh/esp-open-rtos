#pragma once
#ifndef idE6B40052_71B0_4348_A67EB873C93F05DF
#define idE6B40052_71B0_4348_A67EB873C93F05DF

/* Register definitions for the PCM514x DAC converter
 *
 * The device can be configured either through HW, SPI or though I2C
 * Depending on the HW-Mode the pins GPIO have different functions.
 *
 * This header file was taken and adapted from the linux source tree
 *
 */

/* The memory of the PCM is divided into pages. Regsiter 0 on all pages 
 * is the page select register, which controlls where the next read/write
 * will occure.
 */
 
/* The register map 
 *
 *  Page 0:       Control
 *  Page 1:       Analog Control
 *  Page 2-43:    Reserved
 *  Page 44-52:   Coeffient A
 *  Page 53-61:   Reserved
 *  Page 62-70:   Coeffient b
 *  Page 71-151:  Reserved
 *  Page 151-186: Instrution
 *  Page 187-252: Reserved
 *  Page 253:     Clock Flex
 *  Page 254-255: Reserved
 *
 */


#define PCM514x_PAGE_LEN		0x100
#define PCM514x_PAGE_BASE(n)	(PCM514x_PAGE_LEN*n)

#define PCM514x_PAGE(r)		((r >> 8) & 0xFF)

// register 0 on all pages is special
#define PCM514x_PAGE_REG	0x00

// the page registers
#define PCM514x_RESET             (PCM514x_PAGE_BASE(0) +   1)
#define PCM514x_POWER             (PCM514x_PAGE_BASE(0) +   2)
#define PCM514x_MUTE              (PCM514x_PAGE_BASE(0) +   3)
#define PCM514x_PLL_EN            (PCM514x_PAGE_BASE(0) +   4)
#define PCM514x_SPI_MISO_FUNCTION (PCM514x_PAGE_BASE(0) +   6)
#define PCM514x_DSP               (PCM514x_PAGE_BASE(0) +   7)
#define PCM514x_GPIO_EN           (PCM514x_PAGE_BASE(0) +   8)
#define PCM514x_BCLK_LRCLK_CFG    (PCM514x_PAGE_BASE(0) +   9)
#define PCM514x_DSP_GPIO_INPUT    (PCM514x_PAGE_BASE(0) +  10)
#define PCM514x_MASTER_MODE       (PCM514x_PAGE_BASE(0) +  12)
#define PCM514x_PLL_REF           (PCM514x_PAGE_BASE(0) +  13)
#define PCM514x_DAC_REF           (PCM514x_PAGE_BASE(0) +  14)
#define PCM514x_GPIO_PLLIN        (PCM514x_PAGE_BASE(0) +  18)
#define PCM514x_SYNCHRONIZE       (PCM514x_PAGE_BASE(0) +  19)
#define PCM514x_PLL_COEFF_0       (PCM514x_PAGE_BASE(0) +  20)
#define PCM514x_PLL_COEFF_1       (PCM514x_PAGE_BASE(0) +  21)
#define PCM514x_PLL_COEFF_2       (PCM514x_PAGE_BASE(0) +  22)
#define PCM514x_PLL_COEFF_3       (PCM514x_PAGE_BASE(0) +  23)
#define PCM514x_PLL_COEFF_4       (PCM514x_PAGE_BASE(0) +  24)
#define PCM514x_DSP_CLKDIV        (PCM514x_PAGE_BASE(0) +  27)
#define PCM514x_DAC_CLKDIV        (PCM514x_PAGE_BASE(0) +  28)
#define PCM514x_NCP_CLKDIV        (PCM514x_PAGE_BASE(0) +  29)
#define PCM514x_OSR_CLKDIV        (PCM514x_PAGE_BASE(0) +  30)
#define PCM514x_MASTER_CLKDIV_1   (PCM514x_PAGE_BASE(0) +  32)
#define PCM514x_MASTER_CLKDIV_2   (PCM514x_PAGE_BASE(0) +  33)
#define PCM514x_FS_SPEED_MODE     (PCM514x_PAGE_BASE(0) +  34)
#define PCM514x_IDAC_1            (PCM514x_PAGE_BASE(0) +  35)
#define PCM514x_IDAC_2            (PCM514x_PAGE_BASE(0) +  36)
#define PCM514x_ERROR_DETECT      (PCM514x_PAGE_BASE(0) +  37)
#define PCM514x_I2S_1             (PCM514x_PAGE_BASE(0) +  40)
#define PCM514x_I2S_2             (PCM514x_PAGE_BASE(0) +  41)
#define PCM514x_DAC_ROUTING       (PCM514x_PAGE_BASE(0) +  42)
#define PCM514x_DSP_PROGRAM       (PCM514x_PAGE_BASE(0) +  43)
#define PCM514x_CLKDET            (PCM514x_PAGE_BASE(0) +  44)
#define PCM514x_AUTO_MUTE         (PCM514x_PAGE_BASE(0) +  59)
#define PCM514x_DIGITAL_VOLUME_1  (PCM514x_PAGE_BASE(0) +  60)
#define PCM514x_DIGITAL_VOLUME_2  (PCM514x_PAGE_BASE(0) +  61)
#define PCM514x_DIGITAL_VOLUME_3  (PCM514x_PAGE_BASE(0) +  62)
#define PCM514x_DIGITAL_MUTE_1    (PCM514x_PAGE_BASE(0) +  63)
#define PCM514x_DIGITAL_MUTE_2    (PCM514x_PAGE_BASE(0) +  64)
#define PCM514x_DIGITAL_MUTE_3    (PCM514x_PAGE_BASE(0) +  65)
#define PCM514x_GPIO_OUTPUT_1     (PCM514x_PAGE_BASE(0) +  80)
#define PCM514x_GPIO_OUTPUT_2     (PCM514x_PAGE_BASE(0) +  81)
#define PCM514x_GPIO_OUTPUT_3     (PCM514x_PAGE_BASE(0) +  82)
#define PCM514x_GPIO_OUTPUT_4     (PCM514x_PAGE_BASE(0) +  83)
#define PCM514x_GPIO_OUTPUT_5     (PCM514x_PAGE_BASE(0) +  84)
#define PCM514x_GPIO_OUTPUT_6     (PCM514x_PAGE_BASE(0) +  85)
#define PCM514x_GPIO_CONTROL_1    (PCM514x_PAGE_BASE(0) +  86)
#define PCM514x_GPIO_CONTROL_2    (PCM514x_PAGE_BASE(0) +  87)
#define PCM514x_OVERFLOW          (PCM514x_PAGE_BASE(0) +  90)
#define PCM514x_RATE_DET_1        (PCM514x_PAGE_BASE(0) +  91)
#define PCM514x_RATE_DET_2        (PCM514x_PAGE_BASE(0) +  92)
#define PCM514x_RATE_DET_3        (PCM514x_PAGE_BASE(0) +  93)
#define PCM514x_RATE_DET_4        (PCM514x_PAGE_BASE(0) +  94)
#define PCM514x_CLOCK_STATUS      (PCM514x_PAGE_BASE(0) +  95)
#define PCM514x_ANALOG_MUTE_DET   (PCM514x_PAGE_BASE(0) + 108)
#define PCM514x_GPIN              (PCM514x_PAGE_BASE(0) + 119)
#define PCM514x_DIGITAL_MUTE_DET  (PCM514x_PAGE_BASE(0) + 120)

#define PCM514x_OUTPUT_AMPLITUDE  (PCM514x_PAGE_BASE(1) +   1)
#define PCM514x_ANALOG_GAIN_CTRL  (PCM514x_PAGE_BASE(1) +   2)
#define PCM514x_UNDERVOLTAGE_PROT (PCM514x_PAGE_BASE(1) +   5)
#define PCM514x_ANALOG_MUTE_CTRL  (PCM514x_PAGE_BASE(1) +   6)
#define PCM514x_ANALOG_GAIN_BOOST (PCM514x_PAGE_BASE(1) +   7)
#define PCM514x_VCOM_CTRL_1       (PCM514x_PAGE_BASE(1) +   8)
#define PCM514x_VCOM_CTRL_2       (PCM514x_PAGE_BASE(1) +   9)

#define PCM514x_CRAM_CTRL         (PCM514x_PAGE_BASE(44) +  1)

#define PCM514x_FLEX_A            (PCM514x_PAGE_BASE(253) + 63)
#define PCM514x_FLEX_B            (PCM514x_PAGE_BASE(253) + 64)

#define PCM514x_MAX_REGISTER      (PCM514x_PAGE_BASE(253) + 64)

/* Page 0, Register 1 - reset */
#define PCM514x_RSTR (1 << 0)
#define PCM514x_RSTM (1 << 4)

/* Page 0, Register 2 - power */
#define PCM514x_RQPD       (1 << 0)
#define PCM514x_RQPD_SHIFT 0
#define PCM514x_RQST       (1 << 4)
#define PCM514x_RQST_SHIFT 4

/* Page 0, Register 3 - mute */
#define PCM514x_RQMR_SHIFT 0
#define PCM514x_RQML_SHIFT 4

/* Page 0, Register 4 - PLL */
#define PCM514x_PLLE       (1 << 0)
#define PCM514x_PLLE_SHIFT 0
#define PCM514x_PLCK       (1 << 4)
#define PCM514x_PLCK_SHIFT 4

/* Page 0, Register 7 - DSP */
#define PCM514x_SDSL       (1 << 0)
#define PCM514x_SDSL_SHIFT 0
#define PCM514x_DEMP       (1 << 4)
#define PCM514x_DEMP_SHIFT 4

/* Page 0, Register 8 - GPIO output enable */
#define PCM514x_G1OE       (1 << 0)
#define PCM514x_G2OE       (1 << 1)
#define PCM514x_G3OE       (1 << 2)
#define PCM514x_G4OE       (1 << 3)
#define PCM514x_G5OE       (1 << 4)
#define PCM514x_G6OE       (1 << 5)

/* Page 0, Register 9 - BCK, LRCLK configuration */
#define PCM514x_LRKO       (1 << 0)
#define PCM514x_LRKO_SHIFT 0
#define PCM514x_BCKO       (1 << 4)
#define PCM514x_BCKO_SHIFT 4
#define PCM514x_BCKP       (1 << 5)
#define PCM514x_BCKP_SHIFT 5

/* Page 0, Register 12 - Master mode BCK, LRCLK reset */
#define PCM514x_RLRK       (1 << 0)
#define PCM514x_RLRK_SHIFT 0
#define PCM514x_RBCK       (1 << 1)
#define PCM514x_RBCK_SHIFT 1

/* Page 0, Register 13 - PLL reference */
#define PCM514x_SREF        (7 << 4)
#define PCM514x_SREF_SHIFT  4
#define PCM514x_SREF_SCK    (0 << 4)
#define PCM514x_SREF_BCK    (1 << 4)
#define PCM514x_SREF_GPIO   (3 << 4)

/* Page 0, Register 14 - DAC reference */
#define PCM514x_SDAC        (7 << 4)
#define PCM514x_SDAC_SHIFT  4
#define PCM514x_SDAC_MCK    (0 << 4)
#define PCM514x_SDAC_PLL    (1 << 4)
#define PCM514x_SDAC_SCK    (3 << 4)
#define PCM514x_SDAC_BCK    (4 << 4)
#define PCM514x_SDAC_GPIO   (5 << 4)

/* Page 0, Register 16, 18 - GPIO source for DAC, PLL */
#define PCM514x_GREF        (7 << 0)
#define PCM514x_GREF_SHIFT  0
#define PCM514x_GREF_GPIO1  (0 << 0)
#define PCM514x_GREF_GPIO2  (1 << 0)
#define PCM514x_GREF_GPIO3  (2 << 0)
#define PCM514x_GREF_GPIO4  (3 << 0)
#define PCM514x_GREF_GPIO5  (4 << 0)
#define PCM514x_GREF_GPIO6  (5 << 0)

/* Page 0, Register 19 - synchronize */
#define PCM514x_RQSY        (1 << 0)
#define PCM514x_RQSY_RESUME (0 << 0)
#define PCM514x_RQSY_HALT   (1 << 0)

/* Page 0, Register 34 - fs speed mode */
#define PCM514x_FSSP        (3 << 0)
#define PCM514x_FSSP_SHIFT  0
#define PCM514x_FSSP_48KHZ  (0 << 0)
#define PCM514x_FSSP_96KHZ  (1 << 0)
#define PCM514x_FSSP_192KHZ (2 << 0)
#define PCM514x_FSSP_384KHZ (3 << 0)

/* Page 0, Register 37 - Error detection */
#define PCM514x_IPLK (1 << 0)
#define PCM514x_DCAS (1 << 1)
#define PCM514x_IDCM (1 << 2)
#define PCM514x_IDCH (1 << 3)
#define PCM514x_IDSK (1 << 4)
#define PCM514x_IDBK (1 << 5)
#define PCM514x_IDFS (1 << 6)

/* Page 0, Register 40 - I2S configuration */
#define PCM514x_ALEN       (3 << 0)
#define PCM514x_ALEN_SHIFT 0
#define PCM514x_ALEN_16    (0 << 0)
#define PCM514x_ALEN_20    (1 << 0)
#define PCM514x_ALEN_24    (2 << 0)
#define PCM514x_ALEN_32    (3 << 0)
#define PCM514x_AFMT       (3 << 4)
#define PCM514x_AFMT_SHIFT 4
#define PCM514x_AFMT_I2S   (0 << 4)
#define PCM514x_AFMT_DSP   (1 << 4)
#define PCM514x_AFMT_RTJ   (2 << 4)
#define PCM514x_AFMT_LTJ   (3 << 4)

/* Page 0, Register 42 - DAC routing */
#define PCM514x_AUPR	   (3 << 0)
#define PCM514x_AUPR_SHIFT 0
#define PCM514x_AUPL	   (3 << 4)
#define PCM514x_AUPL_SHIFT 4

/* Page 0, Register 59 - auto mute */
#define PCM514x_ATMR       (3 << 0)
#define PCM514x_ATMR_SHIFT 0
#define PCM514x_ATML       (3 << 4)
#define PCM514x_ATML_SHIFT 4

/* Page 0, Register 63 - ramp rates */
#define PCM514x_VNDF       (3 << 6)
#define PCM514x_VNDF_SHIFT 6
#define PCM514x_VNDS       (3 << 4)
#define PCM514x_VNDS_SHIFT 4
#define PCM514x_VNUF       (3 << 2)
#define PCM514x_VNUF_SHIFT 2
#define PCM514x_VNUS       (3 << 0)
#define PCM514x_VNUS_SHIFT 0

/* Page 0, Register 64 - emergency ramp rates */
#define PCM514x_VEDF	   (3 << 6)
#define PCM514x_VEDF_SHIFT 6
#define PCM514x_VEDS	   (3 << 4)
#define PCM514x_VEDS_SHIFT 4

/* Page 0, Register 65 - Digital mute enables */
#define PCM514x_ACTL       (1 << 2)
#define PCM514x_ACTL_SHIFT 2
#define PCM514x_AMLE       (1 << 1)
#define PCM514x_AMLE_SHIFT 1
#define PCM514x_AMRE       (1 << 0)
#define PCM514x_AMRE_SHIFT 0

/* Page 0, Register 80-85, GPIO output selection */
#define PCM514x_GxSL       (31 << 0)
#define PCM514x_GxSL_SHIFT 0
#define PCM514x_GxSL_OFF   (0 << 0)
#define PCM514x_GxSL_DSP   (1 << 0)
#define PCM514x_GxSL_REG   (2 << 0)
#define PCM514x_GxSL_AMUTB (3 << 0)
#define PCM514x_GxSL_AMUTL (4 << 0)
#define PCM514x_GxSL_AMUTR (5 << 0)
#define PCM514x_GxSL_CLKI  (6 << 0)
#define PCM514x_GxSL_SDOUT (7 << 0)
#define PCM514x_GxSL_ANMUL (8 << 0)
#define PCM514x_GxSL_ANMUR (9 << 0)
#define PCM514x_GxSL_PLLLK (10 << 0)
#define PCM514x_GxSL_CPCLK (11 << 0)
#define PCM514x_GxSL_UV0_7 (14 << 0)
#define PCM514x_GxSL_UV0_3 (15 << 0)
#define PCM514x_GxSL_PLLCK (16 << 0)

/* Page 1, Register 2 - analog volume control */
#define PCM514x_RAGN_SHIFT 0
#define PCM514x_LAGN_SHIFT 4

/* Page 1, Register 7 - analog boost control */
#define PCM514x_AGBR_SHIFT 0
#define PCM514x_AGBL_SHIFT 4

#endif // header
