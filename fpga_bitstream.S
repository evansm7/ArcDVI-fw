        .globl fpga_bitstream
fpga_bitstream:
        .incbin "fpga.bit"
fpga_bitstream_end:
        .align
        .globl fpga_bitstream_length
fpga_bitstream_length:
        .long fpga_bitstream_end - fpga_bitstream
        .end
