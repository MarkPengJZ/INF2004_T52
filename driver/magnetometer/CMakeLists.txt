# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add your source file to the magnetometer
add_executable(magnetometer magnetometer.c)

# Link the libraries you need (pico_stdlib and hardware_i2c)
target_link_libraries(magnetometer pico_stdlib hardware_i2c)

# Create a map/binary/hex file etc.
pico_add_extra_outputs(magnetometer)

pico_enable_stdio_usb(magnetometer 1)