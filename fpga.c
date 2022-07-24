/* Program Lattice iCE40 FPGA over SPI
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
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "fpga.h"


#define DEBUG 1

#ifdef DEBUG
#define FDB(x...)       printf(x)
#else
#define FDB(x...)       do {} while(0)
#endif

#define GPIO_INIT_IN(x) do {            \
        gpio_init(x);                   \
        gpio_set_dir(x, GPIO_IN);       \
        } while (0)
#define GPIO_INIT_IN_PU(x) do {         \
        gpio_init(x);                   \
        gpio_set_dir(x, GPIO_IN);       \
        gpio_pull_up(x);                \
        } while (0)
#define GPIO_INIT_OUT(x) do {           \
        gpio_init(x);                   \
        gpio_set_dir(x, GPIO_OUT);      \
        } while (0)

void    fpga_init()
{
        FDB("+++ FPGA init\n");
        /* Set up FPGA clk, 125/2=62.5MHz: */
        clock_gpio_init(MCU_FPGA_CLK, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, 2);

        GPIO_INIT_OUT(MCU_FPGA_nRESET);
        gpio_put(MCU_FPGA_nRESET, 0);                   /* Hold in reset */

        GPIO_INIT_IN(MCU_FPGA_DONE);                    /* Externally pulled */
        GPIO_INIT_IN(MCU_FPGA_IRQ);

        /* SPI initiator to FPGA responder */
        spi_init(spi0, 10*1000000);
        gpio_set_function(MCU_FPGA_SDO, GPIO_FUNC_SPI);
        gpio_set_function(MCU_FPGA_SDI, GPIO_FUNC_SPI);
        gpio_set_function(MCU_FPGA_SCLK, GPIO_FUNC_SPI);

        GPIO_INIT_OUT(MCU_FPGA_SS);
        gpio_put(MCU_FPGA_SS, 0);                       /* Must be 0 at FPGA reset */

        /* FIXME, eventually PIO */
        GPIO_INIT_IN_PU(MCU_FPGA_D0);
        GPIO_INIT_IN_PU(MCU_FPGA_D1);
        GPIO_INIT_IN_PU(MCU_FPGA_D2);
        GPIO_INIT_IN_PU(MCU_FPGA_D3);
        GPIO_INIT_IN_PU(MCU_FPGA_D4);
        GPIO_INIT_IN_PU(MCU_FPGA_D5);
        GPIO_INIT_IN_PU(MCU_FPGA_D6);
        GPIO_INIT_IN_PU(MCU_FPGA_D7);
        GPIO_INIT_IN_PU(MCU_FPGA_STROBE_IN);
        GPIO_INIT_OUT(MCU_FPGA_STROBE_OUT);
        gpio_put(MCU_FPGA_STROBE_OUT, 0);

        FDB("    Done\n");
}

int     fpga_load(uint8_t *bitstream, unsigned int len)
{
        int i;
        uint8_t buff[8];

        FDB("+++ FPGA load, bitstream at %p, len %d\n", bitstream, len);

        FDB("    Resetting FPGA\n");
        gpio_put(MCU_FPGA_SS, 0);                       /* Must be 0 at FPGA reset */
        fpga_reset();
        /* FPGA samples SS after reset and enters config mode */
        sleep_ms(5);

        /* Process is: 8 dummy clocks with SS high, then bitstream bytes with SS
         * low.
         * Then, SS high again and do 100 cycles polling for CDONE=1.
         * Either time-out, or CDONE=1 then send an additional >=49 dummy bits.
         */
        FDB("    8 dummy clock preamble\n");
        gpio_put(MCU_FPGA_SS, 1);
        spi_write_blocking(spi0, (uint8_t *)buff, 1);   /* 8 dummy clocks */

        FDB("    Writing bitstream\n");
        gpio_put(MCU_FPGA_SS, 0);
        spi_write_blocking(spi0, bitstream, len);

        gpio_put(MCU_FPGA_SS, 1);
        for (i = 0; i < 13; i++) {
                FDB("    Waiting for CDONE %d (gpio %d)\n", i, gpio_get(MCU_FPGA_DONE));
                spi_write_blocking(spi0, (uint8_t *)buff, 1);   /* 8C */
                if (fpga_is_ready())
                        break;
        }
        if (i == 13) {
                FDB("*** TIMEOUT on CDONE :(\n");
                return -1;
        }

        FDB("    Final dummy clocks\n");
        for (int i = 0; i < 7; i++) {
                spi_write_blocking(spi0, (uint8_t *)buff, 8);
        }
        FDB("    Done.\n");
        return 0;
}

bool    fpga_is_ready()
{
        return gpio_get(MCU_FPGA_DONE) == 1;
}

void    fpga_reset()
{
        gpio_put(MCU_FPGA_nRESET, 0);
        sleep_ms(1);
        gpio_put(MCU_FPGA_nRESET, 1);
}

uint32_t        fpga_read32(unsigned int addr)
{
        uint8_t packet[4];
        uint32_t rdata;

        packet[0] = (0 << 6) | ((addr >> 6) & 0x3f);
        packet[1] = (addr << 2) & 0xff;
        gpio_put(MCU_FPGA_SS, 0);
        spi_write_blocking(spi0, &packet[0], 2);
        spi_read_blocking(spi0, 0, &packet[0], 4);
        gpio_put(MCU_FPGA_SS, 1);

        rdata = ((uint32_t)packet[0] << 24) | ((uint32_t)packet[1] << 16) |
                ((uint32_t)packet[2] << 8) | packet[3];
        return rdata;
}

void            fpga_write32(unsigned int addr, uint32_t data)
{
        uint8_t packet[6];
        uint32_t rdata;

        packet[0] = (1 << 6) | ((addr >> 6) & 0x3f);
        packet[1] = (addr << 2) & 0xff;
        packet[2] = data >> 24;
        packet[3] = data >> 16;
        packet[4] = data >> 8;
        packet[5] = data;
        gpio_put(MCU_FPGA_SS, 0);
        spi_write_blocking(spi0, &packet[0], 6);
        gpio_put(MCU_FPGA_SS, 1);
}
