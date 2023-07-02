# ArcDVI-fw: Digital video output for the Acorn Archimedes

2 July 2023

ArcDVI is a hardware add-on allowing Acorn Archimedes computers to output video via DVI.  Retrocompute on a _quality_ monitor! :)

The ArcDVI project comprises several parts:

   * The [main PCB](https://github.com/evansm7/ArcDVI-PCB-main), which
     contains an FPGA and a MCU (on which this firmware runs).
   * "VIDC" PCBs: either an [interposer](https://github.com/evansm7/ArcDVI-PCB-VIDC-skt) that holds VIDC in socketed machines (e.g. A440), or a [clip-over](https://github.com/evansm7/ArcDVI-PCB-VIDC) board for machines in which VIDC is soldered down (e.g. A3000)
   * The [FPGA design](https://github.com/evansm7/ArcDVI-hw)
   * The microcontroller [firmware](https://github.com/evansm7/ArcDVI-fw) (__This repo__)
   * Optional extension/test [software](https://github.com/evansm7/ArcDVI-sw) for RISC OS


### Firmware

This firmware is written for the RP2040 microcontroller on the ArcDVI board, and has several duties/features:

   * Initialises the video transmitter/serialiser IC over I2C
     (supports ADV7513, previously TDA19988)
   * Programs the iCE40HX FPGA bitstream
   * Provides a USB CDC debug console, with commands/debug to control & monitor mode changes and video config
   * Provides a register read/write interface to FPGA registers over SPI
   * Monitors the VIDC registers for changes, calculates video output timing and reconfigures the FPGA on the fly

The RP2040 is a fun chip, and the USB bootloader makes it easy for field firmware/bitstream upgrades.

In future, the MCU will also perform audio processing for sound support.


### Safari

Starting at `firmware/main.c`, a simple top-level loop polls registers that indicate if the VIDC `HCR`/`VCR` values have been written (as happens on a mode switch).  If so, `video.c:video_probe_mode()` selects an appropriate output configuration given VIDC's configuration.

Aside from a whole lot of debugging/development features (such as `commands.c` which provides a super-simple CLI to tweak config via UART console), the core responsibility of the firmware is `video_probe_mode()`.


## Building

This project is built using the Raspberry Pi Pico SDK.  Configure it like any other pico-sdk project:

```
[~/ArcDVI-fw]$ mkdir build
[~/ArcDVI-fw]$ cd build
[~/ArcDVI-fw/build]$ cmake .. PICO_SDK_PATH=~/pico-sdk
...(pico-sdk configures)...
```

The FPGA bitstream is embedded into the MCU firmware.  The bitstream must first be built from ArcDVI-hw, and added to the build directory.  The firmware can then be built:

```
[~/ArcDVI-fw/build]$ ln -s ~/ArcDVI-hw/arcdvi-ice40.bit ./fpga.bit
[~/ArcDVI-fw/build]$ make
```

The output is `firmware.uf2`.  This is usually programmed by putting the RP2040 into bootloader mode and copying the file to the resulting USB MSD.



## References

(This link will probably break eventually, but) There's an ArcDVI thread on the StarDot forums with some photos/development notes:  <https://stardot.org.uk/forums/viewtopic.php?f=16&t=23772>


## Copyrights & licence

Copyright 2021-2023 Matt Evans, and provided under the MIT licence.

