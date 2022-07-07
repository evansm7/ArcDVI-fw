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

#include "fpga.h"
#include "video.h"

extern uint8_t fpga_bitstream[];
extern unsigned int fpga_bitstream_length;

int main()
{
	stdio_init_all();

	printf("OHAI\n");

        video_init();
        fpga_init();
        printf("FPGA: Bitstream %d bytes at %p, programming:\n",
               fpga_bitstream_length, fpga_bitstream);
        int r = fpga_load(fpga_bitstream, fpga_bitstream_length);
        printf(" -> Return value %d\n", r);

        int i = 0;
	while (true) {
                printf("Hello %d\n", i++);
                sleep_ms(2500);
	}

	return 0;
}
