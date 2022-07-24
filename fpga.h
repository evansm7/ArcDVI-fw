/*
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

#ifndef FPGA_H
#define FPGA_H

#include <stdint.h>

/* Generic-ish FPGA helpers */

/* Init the FPGA subsystem (e.g. GPIOs, clocks) */
void            fpga_init();
/* Load a bitstream */
int             fpga_load(uint8_t *bitstream, unsigned int len);
/* Test if FPGA configuration is done */
bool            fpga_is_ready();
/* Returns to uninitialised state: */
void            fpga_reset();

/* Payload register I/O */
uint32_t        fpga_read32(unsigned int addr);
void            fpga_write32(unsigned int addr, uint32_t data);

#endif
