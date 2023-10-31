add_executable(irline_barcode
        irline_barcode.c
        )

target_link_libraries(irline_barcode pico_stdlib hardware_adc)

# enable usb output
pico_enable_stdio_usb(irline_barcode 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(irline_barcode)

# add url via pico_set_program_url
example_auto_set_url(irline_barcode)

