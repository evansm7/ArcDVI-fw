/* video_tda19988: initialisation and support for TDA19988 video
 * serialiser.
 *
 * 7 July 2022 ME
 */

#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"
//#include "pico/binary_info.h"
#include "hw.h"
#include "video_tda19988.h"



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

static void     i2c_scan()
{
        printf("--- Bus scan\n");
        for (unsigned int addr = 0; addr < 128; addr++) {
                int r;
                uint8_t rxd;

                printf("%02x: ", addr);

                r = i2c_read_blocking(MCU_VID_I2C, addr, &rxd, 1, false);
                printf(r < 0 ? "--" : "**");

                printf((addr & 7) == 7 ? "\n" : "   ");
        }
        printf("--- Done.");
}

static int      video_reg_write(uint8_t addr, uint8_t reg, uint8_t val)
{
        uint8_t buff[2];
        buff[0] = reg;
        buff[1] = val;
        return i2c_write_blocking(MCU_VID_I2C, addr, buff, 2, false);
}

static void     video_testcard()
{
        /* Basic init for test card */
        video_reg_write(VID_ADDR_CEC, 0xff, 0x87);              /* Enable clocks */
        sleep_ms(50);
        video_reg_write(VID_ADDR_HDMI, TDA_REG_PAGE, 0);        /* Page 0 */
        video_reg_write(VID_ADDR_HDMI, 0xa0, 0);                /* Video format 0 */
        video_reg_write(VID_ADDR_HDMI, 0xe4, 0xc0);             /* Test pattern */
        video_reg_write(VID_ADDR_HDMI, 0xf0, 0x00);             /* RPT_CNTRL */
}

int     video_init()
{
        VDB("+++ video_tda19988 init:\n");

        vid_i2c_init();

        /* OK... now probe some of dem regs */
        video_testcard();

        VDB("    Done\n");
}

int     video_changemode()
{
}

/* Mute I2S audio */
int     video_mute(bool muted)
{
}

/* Misc:
 * - scale?
 * - Gamma?
 * - EDID?
 */
