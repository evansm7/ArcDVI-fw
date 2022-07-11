/* Simple command handler:
 * Provides a trivial CLI, with command handlers/parameters etc.
 *
 * Copyright 2017, 2021, 2022 Matt Evans
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
#include <ctype.h>
#include "pico/stdlib.h"

#include "version.h"
#include "commands.h"
#include "vidc_regs.h"
#include "video.h"


extern uint8_t flag_autoprobe_mode;

#define PROMPT "> "

typedef void (*cmd_fn_t)(char *args);

typedef struct {
        const char *format;
        const char *help;
        cmd_fn_t handler;
} cmd_t;

/******************************************************************************/

void	cmd_init(void)
{
}

/* Look for new activity, basic line editing/dispatch command: */
void    cmd_poll(void)
{
        static char buf[100];
        static unsigned int len = 0;
        static int line_done = 0;

        int r = getchar_timeout_us(10000 /* 10ms */);

        if (r >= 0) {
                char c = (char)r;
                switch (c) {
                case '\r':
                case '\n':      /* Pico stdlib doesn't seem to return \n on USB?? */
                        putchar('\r');
                        putchar('\n');
                        buf[len] = '\0';
                        line_done = 1;
                        break;
                case 8:
                        /* Delete/backspace */
                        if (len > 0)
                        {
                                len--;
                                /* Erase the char: */
                                putchar(8);
                                putchar(' ');
                                putchar(8);
                        }
                        break;
                default:
                        if (len < (sizeof(buf)-1))
                        {
                                buf[len] = c;
                                len++;
                                putchar(c);
                        } /* Else, discard char as the line's too long. */
                }

                if (line_done) {
                        cmd_parse(buf, len);
                        line_done = 0;
                        len = 0;
                        printf(PROMPT);
                }
        }
}

/*****************************************************************************/
/* Utilities */

static char *skipwhitespace(char *str)
{
        while ((*str != '\0') && ((*str == ' ') || (*str == '\t'))) { str++; }
        return str;
}

/* ascii to hex:
 * success = 0 if couldn't parse input (e.g. EOL, or not hex)
 */
static int atoh(char *c, char **end, int *success)
{
        int r = 0;
        int ok = 0;
        char digit = *c;
        char lowerd = tolower(digit);

        while (digit != 0 && ((digit >= '0' && digit <= '9') || (lowerd >= 'a' && lowerd <= 'f'))) {
                int v;
                r = r << 4;
                v = (lowerd >= 'a') ? (lowerd - 'a' + 10) : (lowerd - '0');
                r |= v;
                c++;
                digit = *c;
                lowerd = tolower(digit);
                ok = 1;
        }
        if (ok) {
                *end = c;
        }
        *success = ok;
        return r;
}

static int get_addr_len(char **args, unsigned int *addr, unsigned int *len)
{
        int OKa;

        *addr = atoh(*args, args, &OKa);
        if (!OKa)
        {
                printf("\r\n Syntax error, address expected\r\n");
                return 0;
        }
        *args = skipwhitespace(*args);
        *len = atoh(*args, args, &OKa);
        if (!OKa)
        {
                printf("\r\n Syntax error, len expected\r\n");
                return 0;
        }
        return 1;
}


/*****************************************************************************/
/* Commands */

static void cmd_version(char *args)
{
        printf("     _             ______     _____ \r\n"
               "    / \\   _ __ ___|  _ \\ \\   / /_ _|\r\n"
               "   / _ \\ | '__/ __| | | \\ \\ / / | | \r\n"
               "  / ___ \\| | | (__| |_| |\\ V /  | | \r\n"
               " /_/   \\_\\_|  \\___|____/  \\_/  |___|\r\n");

	printf("  version " BUILD_VERSION " (" BUILD_SHA "), built " BUILD_TIME "\r\n");
        /* FIXME: Dump FPGA version */
}

static void cmd_vtx(char *args)
{
        int OK;
        unsigned int xres, fp, width, bp, wpl;

        xres = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 0\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        fp = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 1\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        width = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 2\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        bp = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 3\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        wpl = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 4\r\n");
                goto fail;
        }

        /* Write video regs: */
        video_set_x_timing(xres, fp, width, bp, wpl);
fail:
        return;
}

static void cmd_vty(char *args)
{
        int OK;
        unsigned int yres, fp, width, bp;

        yres = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 0\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        fp = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 1\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        width = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 2\r\n");
                goto fail;
        }
        args = skipwhitespace(args);
        bp = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 3\r\n");
                goto fail;
        }

        /* Write video regs */
fail:
        video_set_y_timing(yres, fp, width, bp);
}

static void cmd_setmode(char *args)
{
        int OK;
        unsigned int mode;

        mode = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error in arg\r\n");
                return;
        }

        video_setmode(mode);
}

static void cmd_vt(char *args)
{
        video_dump_timing_regs();
}

static void cmd_cursorctrl(char *args)
{
        int OK;
        unsigned int xo;

        xo = atoh(args, &args, &OK);
        if (!OK) {
                printf("\r\n Syntax error, arg 0\r\n");
                goto fail;
        }
        video_set_cursor_x(xo);
fail:
        return;
}

static void cmd_sync(char *args)
{
        video_sync();
}

static void cmd_vidc_dump(char *args)
{
        vidc_dumpregs();
}

static void cmd_autoprobe(char *args)
{
        flag_autoprobe_mode = !flag_autoprobe_mode;
        printf("Autoprobe is %s\r\n", flag_autoprobe_mode ? "on" : "off");
}


/*****************************************************************************/

static void cmd_help(char *args);

static cmd_t commands[] = {
        { .format = "help",
          .help = "help\t\t\t\t\t\tGives this help",
          .handler = cmd_help },
        { .format = "?",
          .help = 0,
          .handler = cmd_help },
        { .format = "ver",
          .help = "ver\t\t\t\t\t\tPrint build version information",
          .handler = cmd_version },
        { .format = "vtx",
          .help = "vtx <xpix> <fp> <sync width> <bp> <dma wpl-1>\tSet X video timing",
          .handler = cmd_vtx },
        { .format = "vty",
          .help = "vty <ypix> <fp> <sync width> <bp>\t\tSet Y video timing",
          .handler = cmd_vty },
        /* FIXME: And set res */
        { .format = "vt",
          .help = "vt\t\t\t\t\t\tDump video timing",
          .handler = cmd_vt },
        { .format = "v",
          .help = "v\t\t\t\t\t\tDump VIDC regs",
          .handler = cmd_vidc_dump },
        { .format = "m",
          .help = "m <mode>\t\t\t\t\tSet mode (arc number)",
          .handler = cmd_setmode },
        { .format = "cc",
          .help = "cc <cursor x offset>\t\t\t\tSet cursor x offset",
          .handler = cmd_cursorctrl },
        { .format = "sync",
          .help = "sync\t\t\t\t\t\tResync display to VIDC",
          .handler = cmd_sync },
        { .format = "a",
          .help = "a\t\t\t\t\t\tToggle mode autoprobing",
          .handler = cmd_autoprobe },
};

static int num_commands = sizeof(commands)/sizeof(cmd_t);

/* Special command */
static void cmd_help(char *args)
{
        printf("\r\n Help:\r\n");
        for (int i = 0; i < num_commands; i++) {
                if (commands[i].help)
                        printf("\t%s\r\n", commands[i].help);
        }
}

void cmd_parse(char *linebuffer, int len)
{
        char *cmd_start;
        int ran_cmd = 0;

        cmd_start = skipwhitespace(linebuffer);

        /* Check for blank line: */
        if (cmd_start - linebuffer == len) {
                return;
        }

        for (int i = 0; i < num_commands; i++) {
                int clen = strlen((char *)commands[i].format);
                if (strncmp(cmd_start, (char *)commands[i].format, clen) == 0) {
                        commands[i].handler(skipwhitespace(cmd_start + clen));
                        ran_cmd = 1;
                        break;
                }
        }
        if (!ran_cmd) {
                printf(" -- Unknown command!\r\n");
                cmd_help(cmd_start);
        }
}
