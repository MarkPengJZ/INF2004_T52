add_executable(magnetometer
magnetometer.c
        )

# pull in common dependencies
target_link_libraries(magnetometer pico_stdlib hardware_i2c hardware_pwm)

# enable PUTTY console debugging :D
pico_enable_stdio_usb(magnetometer 1)
pico_enable_stdio_uart(magnetometer 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(magnetometer)
