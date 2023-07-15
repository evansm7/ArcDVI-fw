/* ArcDVI MCU firmware:  main
 *
 * MIT License
 *
 * Copyright (c) 2022 Matt Evans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "pico/stdlib.h"

#include "version.h"
#include "fpga.h"
#include "hw.h"
#include "dvo.h"
#include "vidc_regs.h"
#include "commands.h"
#include "video.h"


/******************************************************************************/

extern uint8_t fpga_bitstream[];
extern unsigned int fpga_bitstream_length;
uint8_t flag_autoprobe_mode = 1;
uint8_t flag_test_mode = 0;

/******************************************************************************/

/* There are lots of possibilities for configuration sources, such as
 * GPIO DIP switches and flash contents, even USB MSD files.
 *
 * For now, simple GPIO will do:
 */
static void     cfg_init(void)
{
#ifdef MCU_CFG1
        /* Only 1 switch is required at this time. */
        gpio_init(MCU_CFG1);
        gpio_set_dir(MCU_CFG1, GPIO_IN);
        gpio_pull_up(MCU_CFG1);
        gpio_init(MCU_CFG2);
        gpio_set_dir(MCU_CFG2, GPIO_IN);
        gpio_pull_up(MCU_CFG2);
        gpio_init(MCU_CFG3);
        gpio_set_dir(MCU_CFG3, GPIO_IN);
        gpio_pull_up(MCU_CFG3);
        gpio_init(MCU_CFG4);
        gpio_set_dir(MCU_CFG4, GPIO_IN);
        gpio_pull_up(MCU_CFG4);
#endif
}

/* Return config DIP switch values */
uint32_t        cfg_get(void)
{
#ifdef MCU_CFG1
        return (gpio_get(MCU_CFG1) ? 0 : 1) |
                (gpio_get(MCU_CFG2) ? 0 : 2) |
                (gpio_get(MCU_CFG3) ? 0 : 4) |
                (gpio_get(MCU_CFG4) ? 0 : 8);
#else
	return 0;	/* FIXME: Other config methods... */
#endif
}

static void     vidc_config_poll(void)
{
        uint32_t s = fpga_read32(FPGA_VO(VIDO_REG_SYNC));

        int status = !!(s & 8);
        int ack = !!(s & 4);

        if (status != ack) {
                // FIXME: Delay a frame or so, so that all writes have Probably Happened
                printf("<VIDC RECONFIG %08x>\r\n", s);
                fpga_write32(FPGA_VO(VIDO_REG_SYNC), s ^ 4); // Flip ack, enables further detection.

                if (flag_autoprobe_mode)
                        video_probe_mode(false);
        }
}

int main()
{
	stdio_init_all();

	printf("ArcDVI version " BUILD_VERSION " (" BUILD_SHA "), built " BUILD_TIME "\n");

        cmd_init();
        cfg_init();
        fpga_init();

        printf("FPGA: Bitstream %d bytes at %p, programming:\n",
               fpga_bitstream_length, fpga_bitstream);
        int r = fpga_load(fpga_bitstream, fpga_bitstream_length);
        printf(" -> Return value %d\n", r);

        sleep_ms(10);

	if (fpga_read32(FPGA_CTRL(CTRL_ID)) & CTRL_ID_TEST)
		flag_test_mode = 1;

        dvo_init();
        video_init();

	/* If we're in test mode, initialise output to a sane mode: */
	if (flag_test_mode)
		video_set_mode(VMODE_1152);

        /* Active hot-spinning loop to poll various services (monitor regs,
         * interactive UART IO, update OSD, etc.)
         */
        while (1) {
                /* Poll user IO */
                cmd_poll();

		if (!flag_test_mode)
			vidc_config_poll();
        }

	return 0;
}
