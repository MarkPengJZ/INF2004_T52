add_executable(irline_navigation
        irline_navigation.c
        )

target_link_libraries(irline_navigation pico_stdlib hardware_adc)

# enable usb output
pico_enable_stdio_usb(irline_navigation 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(irline_navigation)

# add url via pico_set_program_url
example_auto_set_url(irline_navigation)

