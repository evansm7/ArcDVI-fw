/*
 * Copyright 2023 Matt Evans
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

#ifndef DVO_TDA19988_H
#define DVO_TDA19988_H

/* Register and magic number definitions for ADV7513 */

#define VID_ADDR_MAIN	(0x72/2)
#define VID_ADDR_PKT	(0x70/2)
#define VID_ADDR_EDID	(0x7e/2)
#define VID_ADDR_CEC	(0x78/2)

/* R/O status registers */
#define VIDR_CHIP_REV			0x00
#define VIDR_SPDIF_SAMP_FREQ		0x04	/* Bits 7:4 */
#define VIDR_CTS0			0x04	/* Bits 3:0 */
#define VIDR_CTS1			0x05
#define VIDR_CTS2			0x06
#define VIDR_VIC_RPT_RX			0x3d
#define VIDR_VIC_ACTUAL			0x3e
#define VIDR_VIC_AUX_PROG_INFO		0x3f
#define VIDR_STATUS0			0x42	/* My name: HPD state, monitor sense, I2S mode det */
#define VIDR_PLL_STATUS			0x9e
#define VIDR_ENC_STATUS			0xb8
#define VIDR_DDC_STATUS			0xc8

/* Control regs */
#define VIDR_IO_FORMAT			0x16
#define 	VIDR_IO_FORMAT_422		0x80
#define 	VIDR_IO_FORMAT_DEPTH_12		0x20
#define 	VIDR_IO_FORMAT_DEPTH_10		0x10
#define 	VIDR_IO_FORMAT_DEPTH_8		0x30
#define 	VIDR_IO_FORMAT_DDR_RISING	0x02
#define 	VIDR_IO_FORMAT_BLACK_YCbCr	0x01
#define VIDR_POWER			0x41
#define 	VIDR_POWER_PDOWN		0x40
#define 	VIDR_POWER_RESVD		0x10
#define 	VIDR_POWER_SYNC_ADJ		0x02
#define VIDR_MISC0			0x98
#define 	VIDR_MISC0_VAL			0x03
#define VIDR_MISC1			0x9a
#define 	VIDR_MISC1_VAL			0xe0
#define VIDR_MISC2			0x9c
#define 	VIDR_MISC2_VAL			0x30
#define VIDR_PCLK_DIV			0x9d
#define 	VIDR_PCLK_DIV_RESVD		0x61
#define 	VIDR_PCLK_DIV_2			0x04
#define 	VIDR_PCLK_DIV_4			0x08
#define VIDR_MISC3			0xa2
#define 	VIDR_MISC3_VAL			0xa4
#define VIDR_MISC4			0xa3
#define 	VIDR_MISC4_VAL			0xa4
#define VIDR_HPD_CONTROL		0xd6
#define 	VIDR_HPD_CONTROL_CDC		0x40
#define 	VIDR_HPD_CONTROL_HPD		0x80
#define 	VIDR_HPD_CONTROL_HIGH		0xc0
#define 	VIDR_HPD_CONTROL_TMDS_SOFT	0x10
#define 	VIDR_HPD_CONTROL_AV_GATE	0x01
#define VIDR_MISC5			0xe0
#define 	VIDR_MISC5_VAL			0xd0
#define VIDR_MISC6			0xf9
#define 	VIDR_MISC6_VAL			0x00

#endif
