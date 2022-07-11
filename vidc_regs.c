/* ArcDVI: Acorn VIDC register accessors
 *
 * Copyright 2021-2022 Matt Evans
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
#include <unistd.h>
#include <string.h>
#include "pico/stdlib.h"

#include "fpga.h"
#include "vidc_regs.h"
#include "hw.h"


static const char *modes[] = {
        "Normal", "TM0", "TM1", "TM2"
};

#define REG(x)         fpga_read32(FPGA_VIDC((x)/4))

uint32_t        vidc_reg(unsigned int r)
{
        if (r < 0x100) {
                return REG(r);
        } else {
                return 0;
        }
}

/* Pretty-print the VIDC regs */
void            vidc_dumpregs(void)
{
        /* Palette */
        printf("Palette:\t\t");
        for (int i = 0; i < 16*4; i += 4) {
                printf("%03x ", REG(VIDC_PAL_0 + i));
        }
        printf("\r\n");

        /* Border */
        printf("Border:\t\t\tColour %03x, Hs %d, He %d, Vs %d, Ve %d\r\n",
               REG(VIDC_BORDERCOL),
               (REG(VIDC_H_BORDER_START) >> 14) & 0x3ff,
               (REG(VIDC_H_BORDER_END) >> 14) & 0x3ff,
               (REG(VIDC_V_BORDER_START) >> 14) & 0x3ff,
               (REG(VIDC_V_BORDER_END) >> 14) & 0x3ff);

        /* Cursor */
        printf("Pointer:\t\tColours %03x/%03x/%03x, Hs %d (ext %d), Vs %d, Ve %d\r\n",
               REG(VIDC_CURSORPAL1),
               REG(VIDC_CURSORPAL2),
               REG(VIDC_CURSORPAL3),
               (REG(VIDC_H_CURSOR_START) >> 13) & 0x7ff,
               (REG(VIDC_H_CURSOR_START) >> 11) & 0x3,
               (REG(VIDC_V_CURSOR_START) >> 14) & 0x3ff,
               (REG(VIDC_V_CURSOR_END) >> 14) & 0x3ff);

        /* Display */
        printf("Display Horizontal:\tCycle %d, Sync %d, Dst %d, Dend %d, Ilace %d\r\n",
               (REG(VIDC_H_CYC) >> 14) & 0x3ff,
               (REG(VIDC_H_SYNC) >> 14) & 0x3ff,
               (REG(VIDC_H_DISP_START) >> 14) & 0x3ff,
               (REG(VIDC_H_DISP_END) >> 14) & 0x3ff,
               (REG(VIDC_H_INTERLACE) >> 14) & 0x3ff);

        printf("Display Vertical:\tCycle %d, Sync %d, Dst %d, Dend %d\r\n",
               (REG(VIDC_V_CYC) >> 14) & 0x3ff,
               (REG(VIDC_V_SYNC) >> 14) & 0x3ff,
               (REG(VIDC_V_DISP_START) >> 14) & 0x3ff,
               (REG(VIDC_V_DISP_END) >> 14) & 0x3ff);

        uint32_t ctrl = REG(VIDC_CONTROL);
        printf("Display control:\t%s%s, %sSync, Interlace %s, DMARq %1x, BPP %d, PixClk %d\r\n",
               modes[(ctrl >> 14) & 3],
               (ctrl & 0x100) ? ", TM3" : "",
               (ctrl & 0x80) ? "Composite" : "V",
               (ctrl & 0x40) ? "on" : "off",
               (ctrl >> 4) & 3,
               1 << ((ctrl >> 2) & 3),
               ctrl & 3);

        /* Sound */
        printf("Sound:\t\t\tFreq %d, stereo %1x %1x %1x %1x %1x %1x %1x\r\n",
               REG(VIDC_SOUND_FREQ) & 0xff,
               REG(VIDC_STEREO0) & 0xf,
               REG(VIDC_STEREO1) & 0xf,
               REG(VIDC_STEREO2) & 0xf,
               REG(VIDC_STEREO3) & 0xf,
               REG(VIDC_STEREO4) & 0xf,
               REG(VIDC_STEREO5) & 0xf,
               REG(VIDC_STEREO6) & 0xf,
               REG(VIDC_STEREO7) & 0xf);

        /* Counters: */
        printf("Video DMAs/frame:\t%d\r\nCursor DMAs/frame:\t%d\r\n",
               REG(V_DMAC_VIDEO),
               REG(V_DMAC_CURSOR));

        /* Custom/special regs: */
        printf("Special:\t\t%08x d %08x\r\n",
               REG(VIDC_SPECIAL), REG(VIDC_SPECIAL_DATA));
}
