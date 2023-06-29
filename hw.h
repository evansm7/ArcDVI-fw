/*
 * Copyright 2022 Matt Evans
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef HW_H
#define HW_H

/* MCU GPIOs */

/* FPGA programming/config interface */
#define HW_VERSION		2

#define MCU_FPGA_nRESET         0       /* Out */
#define MCU_FPGA_DONE           1       /* In */
#define MCU_FPGA_IRQ            2       /* In */
#define MCU_FPGA_SDO            3       /* Out */
#define MCU_FPGA_SDI            4       /* In */
#define MCU_FPGA_SS             5       /* Out */
#define MCU_FPGA_SCLK           6       /* Out */
#define MCU_FPGA_D0             7       /* In/out */
#define MCU_FPGA_D1             8       /* In/out */
#define MCU_FPGA_D2             9       /* In/out */
#define MCU_FPGA_D3             10      /* In/out */
#define MCU_FPGA_D4             11      /* In/out */
#define MCU_FPGA_D5             12      /* In/out */
#define MCU_FPGA_D6             13      /* In/out */
#define MCU_FPGA_D7             14      /* In/out */
#define MCU_FPGA_STROBE_IN      15      /* In */
#define MCU_FPGA_STROBE_OUT     16      /* Out */

/* Video serialiser's I2S audio interface */
#define MCU_VID_I2S_WS          17      /* Out */
#define MCU_VID_I2S_CLK         18      /* Out */
#define MCU_VID_I2S_DATA        19      /* Out */

#if (HW_VERSION == 1)
#define MCU_FPGA_CLK            23      /* Out */       /* Clock GPOUT1 */
#define MCU_FPGA_CLK_OUTPUT	CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS

/* Video serialiser config interface */
#define MCU_VID_IRQ             22      /* In */
#define MCU_VID_SDA             28      /* In/out */
#define MCU_VID_SCL             29      /* Out */
#define MCU_VID_I2C             i2c0

/* User config switches */
#define MCU_CFG4                20      /* In */
#define MCU_CFG3                21      /* In */
#define MCU_CFG2                26      /* In */
#define MCU_CFG1                27      /* In */

/* Misc */
#define MCU_A1                  24
#define MCU_A2                  25

#elif (HW_VERSION == 2)

#define MCU_FPGA_CLK            24      /* Out, GPOUT2 */
#define MCU_FPGA_CLK_OUTPUT	CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_VALUE_CLK_SYS

/* MCLK on 20, PWM2A */

/* Video serialiser config interface */
#define MCU_VID_IRQ             21      /* In */
#define MCU_VID_SDA             22      /* In/out */
#define MCU_VID_SCL             23      /* Out */
#define MCU_VID_I2C             i2c1

/* Misc */
#define MCU_A3_ID_DET           29

#endif

#define CFG_SW1                 1

/* FPGA addresses & registers */

#define FPGA_VIDC(x)            (0x000 + (x))
#define FPGA_VO(x)              (0x800 + (x))
#define FPGA_CTRL(x)            (0xc00 + (x))

#define CTRL_ID                 0
#define CTRL_ID_TEST		0x800000	/* Bitstream is a standalone test design */
#define CTRL_REG                1
#define         CR_RESET        0x01
#define         CR_PLL_NRESET   0x02
#define         CR_PLL_CLK      0x04
#define         CR_PLL_DATA     0x08
#define         CR_PLL_DATAO    0x10    /* R/O */
#define         CR_PLL_LOCK     0x20    /* R/O */
#define         CR_PLL_BYPASS   0x40
#define         CR_LED          0x80


extern uint32_t cfg_get();

#endif
