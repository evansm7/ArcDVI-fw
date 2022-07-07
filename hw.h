#ifndef FW_H
#define FW_H

/* MCU GPIOs */

/* FPGA programming/config interface */
#define MCU_FPGA_nRESET         0       /* Out */
#define MCU_FPGA_DONE           1       /* In */
#define MCU_FPGA_IRQ            2       /* In */
#define MCU_FPGA_SDO            3       /* Out */
#define MCU_FPGA_SDI            4       /* In */
#define MCU_FPGA_SS             5       /* Out */
#define MCU_FPGA_SCLK           6       /* Out */
#define MCU_FPGA_D0             7       /* In/out */
#define MCU_FPGA_D1             8       /* In/out */
#define MCU_FPGA_D2             9       /* In/out */
#define MCU_FPGA_D3             10      /* In/out */
#define MCU_FPGA_D4             11      /* In/out */
#define MCU_FPGA_D5             12      /* In/out */
#define MCU_FPGA_D6             13      /* In/out */
#define MCU_FPGA_D7             14      /* In/out */
#define MCU_FPGA_STROBE_IN      15      /* In */
#define MCU_FPGA_STROBE_OUT     16      /* Out */
#define MCU_FPGA_CLK            23      /* Out */       /* Clock GPOUT1 */

/* Video serialiser's I2S audio interface */
#define MCU_VID_I2S_WS          17      /* Out */
#define MCU_VID_I2S_CLK         18      /* Out */
#define MCU_VID_I2S_DATA        19      /* Out */

/* Video serialiser config interface */
#define MCU_VID_IRQ             22      /* In */
#define MCU_VID_SDA             28      /* In/out */
#define MCU_VID_SCL             29      /* Out */
#define MCU_VID_I2C             i2c0

/* User config switches */
#define MCU_CFG4                20      /* In */
#define MCU_CFG3                21      /* In */
#define MCU_CFG2                26      /* In */
#define MCU_CFG1                27      /* In */

/* Misc */
#define MCU_A1                  24
#define MCU_A2                  25

#endif
