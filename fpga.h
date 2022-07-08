#ifndef FPGA_H
#define FPGA_H

#include <stdint.h>

/* Generic-ish FPGA helpers */

/* Init the FPGA subsystem (e.g. GPIOs, clocks) */
void    fpga_init();
/* Load a bitstream */
int     fpga_load(uint8_t *bitstream, unsigned int len);
/* Test if FPGA configuration is done */
bool    fpga_is_ready();
/* Returns to uninitialised state: */
void    fpga_reset();

/* Payload register I/O */
uint32_t        fpga_read32(unsigned int addr);
void            fpga_write32(unsigned int addr, uint32_t data);

#endif
