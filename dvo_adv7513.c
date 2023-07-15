/* dvo_adv7513: initialisation and support for ADV7513 video
 * serialiser.
 *
 * 25 June 2023 ME
 *
 * Copyright 2022-2023 Matt Evans
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

#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hw.h"
#include "dvo.h"
#include "dvo_adv7513.h"


#define DEBUG 1

#ifdef DEBUG
#define VDB(x...)       printf(x)
#else
#define VDB(x...)       do {} while(0)
#endif

#define RR(x)		dvo_reg_read(VID_ADDR_MAIN, x)

static void     vid_i2c_init()
{
        i2c_init(MCU_VID_I2C, 100*1000);
        gpio_set_function(MCU_VID_SDA, GPIO_FUNC_I2C);
        gpio_set_function(MCU_VID_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(MCU_VID_SDA);      /* Elides external P/U? */
        gpio_pull_up(MCU_VID_SCL);
}

static int      dvo_reg_write(uint8_t addr, uint8_t reg, uint8_t val)
{
        uint8_t buff[2];
        buff[0] = reg;
        buff[1] = val;
        return i2c_write_blocking(MCU_VID_I2C, addr, buff, 2, false);
}

/* Returns 0-ff for byte, -1 for error */
static int	dvo_reg_read(uint8_t addr, uint8_t reg)
{
	int r;
	uint8_t rxd;
        uint8_t buff[2];
        buff[0] = reg;
        r = i2c_write_blocking(MCU_VID_I2C, addr, buff, 1, true); // No stop
	if (r < 0)
		return -1;
	r = i2c_read_blocking(MCU_VID_I2C, addr, &rxd, 1, false);
	return r < 0 ? -1 : rxd;
}

static void     i2c_scan()
{
        printf("--- Bus scan\r\n");
        for (unsigned int addr = 0; addr < 128; addr++) {
                int r;
                uint8_t rxd;

                printf("%02x: ", addr);

                r = i2c_read_blocking(MCU_VID_I2C, addr, &rxd, 1, false);
                printf(r < 0 ? "--" : "**");

                printf((addr & 7) == 7 ? "\r\n" : "   ");
        }
        printf("--- Done.\r\n");
}

int     dvo_init_output()
{
	i2c_scan();

	VDB(" HW rev 0x%02x\r\n", dvo_reg_read(VID_ADDR_MAIN, VIDR_CHIP_REV));

	/* From the manual's 'quick start' init sequence: */
	dvo_reg_write(VID_ADDR_MAIN, VIDR_POWER, VIDR_POWER_RESVD);
	sleep_ms(10);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC0, VIDR_MISC0_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC1, VIDR_MISC1_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC2, VIDR_MISC2_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_PCLK_DIV, VIDR_PCLK_DIV_RESVD);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC3, VIDR_MISC3_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC4, VIDR_MISC4_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC5, VIDR_MISC5_VAL);
	dvo_reg_write(VID_ADDR_MAIN, VIDR_MISC6, VIDR_MISC6_VAL);

	dvo_reg_write(VID_ADDR_MAIN, 0x16, 0x30);

        return 0;
}

int     dvo_init()
{
        VDB("+++ DVO adv7513 init:\r\n");

        vid_i2c_init();

        /* OK... now probe some of dem regs */
        dvo_init_output();

        VDB("    Done\r\n");
}

/* Mute I2S audio */
int     dvo_mute(bool muted)
{
}

int	dvo_status()
{
	/* Dump regs */
        printf("--- ADV7513 reg space:\r\n");
        for (unsigned int addr = 0; addr < 0x100; addr++) {
                int r;
                uint8_t rxd;

                printf("%02x: ", addr);

		rxd = RR(addr);
                printf("%02x ", rxd);

                printf((addr & 15) == 15 ? "\r\n" : "  ");
        }
        printf("--- CTS %06x, SPDIF_SAMP %x\r\n",
	       ((unsigned int)(RR(VIDR_CTS0) & 0x0f) << 16) |
	       ((unsigned int)RR(VIDR_CTS1) << 8) |
	       RR(VIDR_CTS2),
	       RR(VIDR_CTS0) >> 4);
        printf("--- VIC_RPT_RX %02x, VIC_ACTUAL %02x, VIC_AUX_PROG_INFO %02x\r\n",
	       RR(VIDR_VIC_RPT_RX), RR(VIDR_VIC_ACTUAL), RR(VIDR_VIC_AUX_PROG_INFO));
        printf("--- STATUS0 %02x, PLL %02x, ENC %02x, DDC %02x\r\n",
	       RR(VIDR_STATUS0), RR(VIDR_PLL_STATUS), RR(VIDR_ENC_STATUS), RR(VIDR_DDC_STATUS));

	return 0;
}
