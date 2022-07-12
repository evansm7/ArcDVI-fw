/* ArcDVI video output control
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
#include "video.h"
#include "hw.h"

#define VR(x)           fpga_read32(FPGA_VO(x))
#define VW(x, val)      fpga_write32(FPGA_VO(x), val)

void    video_sync(void)
{
        uint32_t s = VR(VIDO_REG_SYNC);
        printf("Sync reg: %02x\r\nRequesting sync...", s);
        VW(VIDO_REG_SYNC, s ^ 1);
        int t = 1000000;
        do {
                s = VR(VIDO_REG_SYNC);
                if ((s & 1) == ((s >> 1) & 1)) {
                        printf("Synchronised (new reg %02x)\r\n", s);
                        return;
                }
        } while (--t > 0);
        printf("Timeout :(  (reg %02x)\r\n", s);
}

/* These modes are largely incorrect, but were useful for playing around with variants.
 */
void    video_setmode(int mode)
{
        unsigned int xres, xfp, xwidth, xbp;
        unsigned int yres, yfp, ywidth, ybp;
        unsigned int wpl;
        unsigned int cx;
        unsigned int bpp;
        unsigned int hires = 0;
        unsigned int dx = 0, dy = 0;

        switch (mode) {
        case 23: { // Equivalent to mode 23 (see comments in RTL, 78MHz pclk => 1274 total width
                xres = 1152;    xfp = 40;       xwidth = 20;    xbp = 1274-xres-xfp-xwidth;
                yres = 896;     yfp = 4;        ywidth = 3;     ybp = 950-yres-yfp-ywidth;
                wpl = 36-1;     cx = 0x12c;     hires = 1;
                bpp = 0;
        } break;

        case 25:
        case 26:
        case 27:
        case 28: { // ~Exactly matches mode 25 (24MHz pclk, 1bpp)
                xres = 640;     xfp = 34;       xwidth = 96;    xbp = 800-xres-xfp-xwidth;
                yres = 480;     yfp = 11;       ywidth = 1;     ybp = 525-yres-yfp-ywidth;
                // This magic pointer offset seems constant across BPP, and HDSR value...
                cx = 137; // Note, this is ((HDSR*2)+19)-6

                bpp = mode-25; // 0-3 for 1bpp-8bpp
                wpl = (640/(32>>bpp))-1;
        } break;

        case 21:
        case 20:
        case 19:
        case 18: { // 24MHz
                xres = 640;     xfp = 87;       xwidth = 56;    xbp = 896-xres-xfp-xwidth;
                yres = 512;     yfp = 1;        ywidth = 3;     ybp = 534-yres-yfp-ywidth;
                bpp = mode-18; // 0-3 for 1bpp-8bpp
                wpl = (640/(32>>bpp))-1;
                cx = (0x52*2)+5-6;
        } break;

        case 0:
        case 8:
        case 12:
        case 15: { // 24MHz, 640x256 line-doubled
                if (mode == 0) bpp = 0;
                else if (mode == 8) bpp = 1;
                else if (mode == 12) bpp = 2;
                else bpp = 3;
                /* This mode is very odd-shaped.  We're trying to match exactly 50% of the
                 * horiz period the arc gives out, except arc uses 16MHz pclk and we're
                 * using 24MHz.  So, we want a frame 768 wide instead of 1024 (a la mode 20)
                 * and make the frame very tall (624, 2x 312 lines).
                 */
                xres = 640;     xfp = 40;       xwidth = 20;    xbp = 768-xres-xfp-xwidth;
                yres = 512;     yfp = 40;       ywidth = 5;     ybp = 624-yres-yfp-ywidth;
                wpl = (640/(32>>bpp))-1;
                cx = (0x6d*2)+5-6;
                dy = 1;
        } break;

        case 4:
        case 1: // or 5
        case 9:
        case 13:
        case 0xcc: { // 24MHz, 320x256 line-doubled, pixel-doubled
                if (mode == 4) bpp = 0;
                else if (mode == 1) bpp = 1;
                else if (mode == 9) bpp = 2;
                else if (mode == 13) bpp = 3;
                else if (mode == 0xcc) bpp = 4; // Magic HICOLOUR 565 mode
                /* This mode is very odd-shaped.  We're trying to match exactly 50% of the
                 * horiz period the arc gives out, except arc uses 16MHz pclk and we're
                 * using 24MHz.  So, we want a frame 768 wide instead of 1024 (a la mode 20)
                 * and make the frame very tall (624, 2x 312 lines).
                 */
                xres = 640;     xfp = 40;       xwidth = 20;    xbp = 768-xres-xfp-xwidth;
                yres = 512;     yfp = 40;       ywidth = 5;     ybp = 624-yres-yfp-ywidth;
                wpl = (320/(32>>bpp))-1;
                cx = 0x68; // shrug
                dx = 1;
                dy = 1;
        } break;

        default:
                printf("\r\n Unknown mode!\r\n");
                return;
        }

        // Set mode, and sync:

        VW(VIDO_REG_RES_X, xres | (dx ? 0x80000000 : 0));
        VW(VIDO_REG_HS_FP, xfp);
        VW(VIDO_REG_HS_WIDTH, xwidth);
        VW(VIDO_REG_HS_BP, xbp);
        VW(VIDO_REG_RES_Y, yres | (dy ? 0x80000000 : 0));
        VW(VIDO_REG_VS_FP, yfp);
        VW(VIDO_REG_VS_WIDTH, ywidth);
        VW(VIDO_REG_VS_BP, ybp);
        VW(VIDO_REG_WPLM1, wpl);
        VW(VIDO_REG_CTRL, cx | (hires ? 0x80000000 : 0) | (bpp << 28));

        video_sync();
}

void    video_wait_flybk(void)
{
        /* This waits for a 1-to-0 transition of flyback.
         * Depending on CPU speed/interrupts/whatever, this might
         * wait longer than a frame!
         */
        uint32_t s;

        // Wait for a 1 (might exit immediately):
        do {
                s = VR(VIDO_REG_SYNC);
        } while (!(s & 0x10));

        // Wait for a 0:
        do {
                s = VR(VIDO_REG_SYNC);
        } while (s & 0x10);
}

static int      video_guess_hires(unsigned int x, unsigned int y, unsigned int bpp,
                                  unsigned int pclk)
{
        // Try to guess if this is a hires mode, by very tall high-clock 4BPP modes:
        return (pclk == 24) && (bpp == 2) && (x < (y/2));
}

void    video_probe_mode(void)
{
        const unsigned int pix_rates[] = { 8, 12, 16, 24 };

        video_wait_flybk();

        static unsigned int prev_xres = ~0;
        static unsigned int prev_yres = ~0;
        static unsigned int prev_xfp = ~0;
        static unsigned int prev_xsw = ~0;
        static unsigned int prev_xbp = ~0;
        static unsigned int prev_yfp = ~0;
        static unsigned int prev_ysw = ~0;
        static unsigned int prev_ybp = ~0;
        static unsigned int prev_wpl = ~0;

        // fp is dispend to frame (sync start)
        // bo is dispstart-syncwidth
        unsigned int cr = vidc_reg(VIDC_CONTROL);
        unsigned int bpp = (cr >> 2) & 3;
        unsigned int pix_rate = pix_rates[(cr & 3)];
        unsigned int hcr = ((vidc_reg(VIDC_H_CYC) >> 14)*2)+2;
        unsigned int hsw = ((vidc_reg(VIDC_H_SYNC) >> 14)*2)+2;
        unsigned int hdsr = ((vidc_reg(VIDC_H_DISP_START) >> 14)*2) +
                vidc_bpp_to_hdsr_offset(bpp);
        unsigned int hder = ((vidc_reg(VIDC_H_DISP_END) >> 14)*2) +
                vidc_bpp_to_hdsr_offset(bpp);
        unsigned int vcr = (vidc_reg(VIDC_V_CYC) >> 14)+1;
        unsigned int vsw = (vidc_reg(VIDC_V_SYNC) >> 14)+1;
        unsigned int vdsr = (vidc_reg(VIDC_V_DISP_START) >> 14)+1;
        unsigned int vder = (vidc_reg(VIDC_V_DISP_END) >> 14)+1;

        unsigned int xres = hder - hdsr;
        unsigned int yres = vder - vdsr;
        unsigned int xfp = hcr - hder;
        unsigned int xsw = hsw;
        unsigned int xbp = hdsr - hsw;
        unsigned int yfp = vcr - vder;
        unsigned int ysw = vsw;
        unsigned int ybp = vdsr - vsw;
        unsigned int wpl = (xres/(32>>bpp))-1;
        unsigned int cx = hdsr - 6;
        unsigned int hires = 0;
        unsigned int dx = 0, dy = 0;

        if (xres != prev_xres || yres != prev_yres ||
            xfp != prev_xfp || xsw != prev_xsw || xbp != prev_xbp ||
            yfp != prev_yfp || ysw != prev_ysw || ybp != prev_ybp ||
            wpl != prev_wpl) {
                printf("New mode %dx%d, %dbpp:\r\n"
                       "\thfp %d, hsw %d, hbp %d (%d total)\r\n"
                       "\tvfp %d, vsw %d, vbp %d (%d total, frame %dHz pclk %dMHz)\r\n",
                       xres, yres, 1 << bpp,
                       xfp, xsw, xbp, xres + xfp + xsw + xbp,
                       yfp, ysw, ybp, yres + yfp + ysw + ybp,
                       pix_rate*1000000 / (hcr * vcr), pix_rate);

                prev_xres = xres;
                prev_yres = yres;
                prev_xfp = xfp;
                prev_xsw = xsw;
                prev_xbp = xbp;
                prev_yfp = yfp;
                prev_ysw = ysw;
                prev_ybp = ybp;
                prev_wpl = wpl;
        } else {
                /* Don't reprogram the video output unless we're really doing something different,
                 * because the monitor will spend a second or two to regain sync and
                 * bootup messages will be missed.
                 */
                printf("Config changed, but equals existing mode %dx%d, %dbpp:\r\n"
                       "\thfp %d, hsw %d, hbp %d (%d total)\r\n"
                       "\tvfp %d, vsw %d, vbp %d (%d total, frame %dHz pclk %dMHz)\r\n",
                       xres, yres, 1 << bpp,
                       xfp, xsw, xbp, xres + xfp + xsw + xbp,
                       yfp, ysw, ybp, yres + yfp + ysw + ybp,
                       pix_rate*1000000 / (hcr * vcr), pix_rate);

                return;
        }

        // Now, some dumb heuristics to try to program a matching output mode:
        // 1. Is it a highres mode?
        // 2. Is it a regular VGA/mode21-like mode?
        // 3. Otherwise, something needs doubling.

        if (video_guess_hires(xres, yres, bpp, pix_rate)) {
                /* Not totally infallible, but definitely works for mode 23 ;-)
                 * Hopefully this will work for x900 variants.
                 */
                printf("Guessed hires mono mode.\r\n");

                xres *= 4;
#ifdef LEGACY_78M_CLEVER_HACK_FOR_POSTERITY
                // Recalculate horizontal timing to match same period for 78MHz vs 96MHz:
                unsigned int total_width78 = hcr*4*78/96;

                // Check if that was an integer...
                if (total_width78*96/78/4 != hcr) {
                        printf("*** Cannot match HR horizontal period! "
                                "(orig width %d, new width %d) ***\r\n",
                                hcr, total_width78);
                        // Oh well.  Carry on and see what it looks like.  ;)
                }
                xfp = total_width78 / 20;
                xsw = total_width78 / 40;
                xbp = total_width78-xres-xfp-xsw;
#else
                /* ArcDVI can do a 96MHz pixel clock, so output VIDC/RISC OS timings
                 * directly.  Whether your monitor likes 'em is another matter, as they're
                 * not quite VESA, but "works for me".
                 */
                xfp *= 4;
                xsw *= 4;
                xbp *= 4;
#endif
                // vertical timing stays the same.
                hires = 1;
                bpp = 0;
                wpl = (xres/32)-1;

                cx = 0x12c; // FIXME: derive this from ... something! ;(

        } else if (xres >= 640 && yres >= 480) {
                // Defaults above are good!

        } else if (xres >= 640 && yres < 480) {
                /* We'll want some Y doublin'.  Slightly more complicated now,
                 * because we need to recalculate the horiz timing to fit a 24MHz pclk
                 * instead of the input one.  Specifically, we output the line twice
                 * but need to keep the same vertical timing, which means we need to output
                 * it twice as fast horizontally.  This is done by always using a 24MHz
                 * output clock and changing the blanking time.
                 *
                 * Note, not all modes will succeed here:
                 * - Maybe the mode is already using a 24MHz pclk (e.g. mode 37),
                 *   meaning we can't output the input line at 2x the rate
                 * - Or, the mode is <24 (e.g. 16MHz) but so wide that we can't output
                 *   enough pixels at 24MHz to keep within the line period.
                 *
                 * The fallback is outputting the mode non-doubled, which will likely
                 * not work (monitors/TVs seem to like 400-ish lines at a minimum).
                 *
                 * Longer-term FIXME by changing to a higher output pclk (e.g. 48MHz).
                 */

                /* We need exactly 1/2 of the original line period, but with a
                 * 24MHz clock:
                 */
                unsigned int new_total_width = hcr*24/pix_rate/2;

                /* Being too skimpy on H-blank time upsets many monitors, so
                 * refuse to go into such a mode:
                 */
                unsigned int minimum_h_blanking = xres / 32; // Art not science

                if (pix_rate == 24 || (new_total_width < (xres + minimum_h_blanking))) {
                        printf("*** Can't line-double this mode! "
                                "(%d MHz, width %d (min %d) ***\r\n",
                                pix_rate, new_total_width, xres + minimum_h_blanking);
                        /* Fall through, and set this mode verbatim, maybe
                         * the display can cope with it directly.
                         */
                } else {

                        yres *= 2;
                        yfp *= 2;
                        ysw *= 2;
                        ybp *= 2;

                        // Synthesise new sync parameters using roughly a 2:1:4 ratio:
                        xfp = new_total_width / 20;
                        xsw = new_total_width / 40;
                        xbp = new_total_width-xres-xfp-xsw;

                        printf("Y-doubled: new width %d, fp %d, xsw %d, bp %d\r\n",
                                new_total_width, xfp, xsw, xbp);
                        dy = 1;
                }
        } else if (xres < 640 && yres < 480) {
                /* We'll want both X and Y doublin'.  This'll generally work unless
                 * the mode is a weird custom almost-VGA mode, at 24MHz:
                 */
                unsigned int new_total_width = hcr*24/pix_rate/2;

                if (pix_rate == 24) {
                        printf("*** Can't line-double this 24MHz mode! ***\r\n");
                } else {
                        wpl = (xres/(32>>bpp))-1;

                        yres *= 2;
                        yfp *= 2;
                        ysw *= 2;
                        ybp *= 2;

                        // Synthesise new sync parameters using roughly a 2:1:4 ratio:
                        xres *= 2;
                        xfp = new_total_width / 20;
                        xsw = new_total_width / 40;
                        xbp = new_total_width-xres-xfp-xsw;

                        printf("XY-doubled: new width %d, fp %d, xsw %d, bp %d\r\n",
                                new_total_width, xfp, xsw, xbp);
                        dx = 1;
                        dy = 1;
                }
        }

        VW(VIDO_REG_RES_X, xres | (dx ? 0x80000000 : 0));
        VW(VIDO_REG_HS_FP, xfp);
        VW(VIDO_REG_HS_WIDTH, xsw);
        VW(VIDO_REG_HS_BP, xbp);
        VW(VIDO_REG_RES_Y, yres | (dy ? 0x80000000 : 0));
        VW(VIDO_REG_VS_FP, yfp);
        VW(VIDO_REG_VS_WIDTH, ysw);
        VW(VIDO_REG_VS_BP, ybp);
        VW(VIDO_REG_WPLM1, wpl);
        VW(VIDO_REG_CTRL, cx | (hires ? 0x80000000 : 0) | (bpp << 28));

        video_sync();
}

void    video_dump_timing_regs(void)
{
        uint32_t ctrl = VR(VIDO_REG_CTRL);
        printf("Video timing regs:\r\n"
                " X width 0x%x, front porch 0x%x, width 0x%x, back porch 0x%x, "
                "DMA words per line-1 0x%x\r\n"
                " Y height 0x%x, front porch 0x%x, width 0x%x, back porch 0x%x\r\n"
                " Cursor X offset 0x%x, BPP %d, hires %d\r\n",
                VR(VIDO_REG_RES_X), VR(VIDO_REG_HS_FP), VR(VIDO_REG_HS_WIDTH),
                VR(VIDO_REG_HS_BP), VR(VIDO_REG_WPLM1),
                VR(VIDO_REG_RES_Y), VR(VIDO_REG_VS_FP), VR(VIDO_REG_VS_WIDTH),
                VR(VIDO_REG_VS_BP), ctrl & 0x7ff, 1 << ((ctrl >> 28) & 7),
                !!(ctrl & 0x80000000)
                );
}

void    video_set_x_timing(unsigned int xres, unsigned int fp, unsigned int sw,
                           unsigned int bp, unsigned int wpl)
{
        VW(VIDO_REG_RES_X, xres);
        VW(VIDO_REG_HS_FP, fp);
        VW(VIDO_REG_HS_WIDTH, sw);
        VW(VIDO_REG_HS_BP, bp);
        VW(VIDO_REG_WPLM1, wpl);
}

void    video_set_y_timing(unsigned int yres, unsigned int fp, unsigned int sw,
                           unsigned int bp)
{
        VW(VIDO_REG_RES_Y, yres);
        VW(VIDO_REG_VS_FP, fp);
        VW(VIDO_REG_VS_WIDTH, sw);
        VW(VIDO_REG_VS_BP, bp);
}

void    video_set_cursor_x(unsigned int offset)
{
        VW(VIDO_REG_CTRL, (VR(VIDO_REG_CTRL) & ~0x7ff) | (offset & 0x7ff));
}
