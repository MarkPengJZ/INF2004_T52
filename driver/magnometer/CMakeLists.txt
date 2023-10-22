cmake_minimum_required(VERSION 3.12)


# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# Add your source file to the project
add_executable(magnometer magnometer.c)

# Link the libraries you need (pico_stdlib and hardware_i2c)
target_link_libraries(magnometer pico_stdlib hardware_i2c)

# Create a map/binary/hex file etc.
pico_add_extra_outputs(magnometer)

pico_enable_stdio_usb(magnometer 1)