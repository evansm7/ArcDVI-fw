#ifndef FPGA_H
#define FPGA_H

#include <stdint.h>

/* Generic-ish FPGA helpers */

/* Init the FPGA subsystem (e.g. GPIOs, clocks) */
void    fpga_init();
/* Load a bitstream */
void    fpga_load(uint8_t *bitstream, unsigned int len);
/* Test if FPGA configuration is done */
bool    fpga_is_ready();
/* Returns to uninitialised state: */
void    fpga_reset();

#endif
