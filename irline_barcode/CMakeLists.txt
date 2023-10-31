add_executable(ir_barcode
        ir_barcode.c
        )

target_link_libraries(ir_barcode pico_stdlib hardware_adc)

# enable usb output
pico_enable_stdio_usb(ir_barcode 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ir_barcode)

# add url via pico_set_program_url
example_auto_set_url(ir_barcode)

