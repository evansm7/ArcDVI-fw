# CMakeLists
#
# MIT License
#
# Copyright (c) 2022 Matt Evans
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#
cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(firmware)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

if (TARGET tinyusb_device)
  add_executable(firmware
    main.c
    fpga.c
    # In future, conditionally include a driver corresponding to HW reqs:
    video_tda19988.c
    )

  target_link_libraries(firmware pico_stdlib hardware_i2c hardware_spi)
  # enable usb output, disable uart output
  pico_enable_stdio_usb(firmware 1)
  pico_enable_stdio_uart(firmware 0)

  # Needed for UF2:
  pico_add_extra_outputs(firmware)

elseif(PICO_ON_DEVICE)
   message(WARNING "not building firmware because TinyUSB submodule is not initialized in the SDK")
endif()

