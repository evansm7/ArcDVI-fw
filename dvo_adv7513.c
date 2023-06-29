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

	VDB(" HW rev 0x%02x\r\n", dvo_reg_read(VID_ADDR_MAIN, 0));

	/* From the manual's 'quick start' init sequence: */
	dvo_reg_write(VID_ADDR_MAIN, 0x41, 0x10); // power down = 0

	dvo_reg_write(VID_ADDR_MAIN, 0x98, 0x03);
	dvo_reg_write(VID_ADDR_MAIN, 0x9a, 0xe0);
	dvo_reg_write(VID_ADDR_MAIN, 0x9c, 0x30);
	dvo_reg_write(VID_ADDR_MAIN, 0x9d, 0x01);
	dvo_reg_write(VID_ADDR_MAIN, 0xa2, 0xa4);
	dvo_reg_write(VID_ADDR_MAIN, 0xa3, 0xa4);
	dvo_reg_write(VID_ADDR_MAIN, 0xe0, 0xd0);
	dvo_reg_write(VID_ADDR_MAIN, 0xf9, 0x00);

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
