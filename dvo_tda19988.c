/* dvo_tda19988: initialisation and support for TDA19988 video
 * serialiser.
 *
 * 7 July 2022 ME
 *
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

#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hw.h"
#include "dvo.h"
#include "dvo_tda19988.h"


/* #define DEBUG 1 */

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

static void     dvo_testcard()
{
        /* Basic init for test card */
        dvo_reg_write(VID_ADDR_CEC, 0xff, 0x87);              /* Enable clocks */
        sleep_ms(50);
        dvo_reg_write(VID_ADDR_HDMI, TDA_REG_PAGE, 0);        /* Page 0 */
        dvo_reg_write(VID_ADDR_HDMI, 0xa0, 0);                /* Video format 0 */
        dvo_reg_write(VID_ADDR_HDMI, 0xe4, 0xc0);             /* Test pattern */
        dvo_reg_write(VID_ADDR_HDMI, 0xf0, 0x00);             /* RPT_CNTRL */
}

int     dvo_init_output()
{
        // Set up VGA, "putthrough" mode
        dvo_reg_write(VID_ADDR_CEC, 0xff, 0x87);              /* Enable clocks */
        sleep_ms(50);
        dvo_reg_write(VID_ADDR_HDMI, TDA_REG_PAGE, 0);        /* Page 0 */

        dvo_reg_write(VID_ADDR_HDMI, 0xe4, 0);                /* Test mode off */
        dvo_reg_write(VID_ADDR_HDMI, 0xf0, 0x00);             /* RPT_CNTRL */

        unsigned int c_refpix;
        unsigned int c_refline;
        unsigned int c_xpix, c_ypix;
        unsigned int c_vs_start, c_vs_start_pixel, c_vs_end, c_vs_end_pixel;
        unsigned int c_hs_start_pixel;
        unsigned int c_vwin_start, c_vwin_end;
        unsigned int c_de_start_pixel, c_de_end_pixel;

        /* These work for plain VGA (hfp 16, hsw 96, hbp 44, vfp 10, vsw 2, vbp 31),
         * but also seem to work for 640x512 with the same porch sizes...!
         *
         * errrrrrrrr.... 800x600 also works.  WTAF, why does the TDA even need these
         * things programmed?  (Nobody else requires it :P )
         * 1152x896@50Hz (125MHz pclk) also works, with the exact same parameters.
         *
         * These magic resolution numbers are inspired by the NXP-published
         * LPC example code.
         */
        c_refpix = 857;
        c_refline = 4;
        c_xpix = 798;
        c_ypix = 555;
        c_vs_start = 10;
        c_vs_start_pixel = 36;
        c_vs_end = 12;
        c_vs_end_pixel = 36;
        c_hs_start_pixel = 16;
        c_vwin_start = 36;
        c_vwin_end = 555;
        c_de_start_pixel = 160;
        c_de_end_pixel = 800;

        dvo_reg_write(VID_ADDR_HDMI, 0xa0, 0);                /* Video format 0 = VGA */

        dvo_reg_write(VID_ADDR_HDMI, 0xa1, c_refpix >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xa2, c_refpix & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xa3, c_refline >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xa4, c_refline & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xa5, c_xpix >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xa6, c_xpix & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xa7, c_ypix >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xa8, c_ypix & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xa9, c_vs_start >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xaa, c_vs_start & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xab, c_vs_start_pixel >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xac, c_vs_start_pixel & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xad, c_vs_end >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xae, c_vs_end & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xaf, c_vs_end_pixel >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xb0, c_vs_end_pixel & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xb9, c_hs_start_pixel >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xba, c_hs_start_pixel & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xbd, c_vwin_start >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xbe, c_vwin_start & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xbf, c_vwin_end >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xc0, c_vwin_end & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xc5, c_de_start_pixel >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xc6, c_de_start_pixel & 0xff);
        dvo_reg_write(VID_ADDR_HDMI, 0xc7, c_de_end_pixel >> 8);
        dvo_reg_write(VID_ADDR_HDMI, 0xc8, c_de_end_pixel & 0xff);

        dvo_reg_write(VID_ADDR_HDMI, 0xcb, 0x7c);             /* TBG_CNTRL1 ??? */
        dvo_reg_write(VID_ADDR_HDMI, 0x20, 0x23);             /* swapA=2, swapB=3 */
        dvo_reg_write(VID_ADDR_HDMI, 0x21, 0x45);             /* swapC=4, swapD=5 */
        dvo_reg_write(VID_ADDR_HDMI, 0x22, 0x01);             /* swapE=0, swapF=1 */
        dvo_reg_write(VID_ADDR_HDMI, 0x23, 0x20);             /* VIP_CNTRL3 SYNC_HS=1 */

        dvo_reg_write(VID_ADDR_HDMI, 0xe5, 0x10);             /* HVF_CNTRL1 pad 1 (??) */

        return 0;
}

int     dvo_init()
{
        VDB("+++ video_tda19988 init:\n");

        vid_i2c_init();

        /* OK... now probe some of dem regs */
//        dvo_testcard();
        dvo_init_output();

        VDB("    Done\n");
}

/* Mute I2S audio */
int     dvo_mute(bool muted)
{
}

/* Misc:
 * - scale?
 * - Gamma?
 * - EDID?
 */
