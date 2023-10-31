add_subdirectory(encoder)
add_subdirectory(motor)
add_subdirectory(ultrasonic)

add_executable(demo controller.c)

pico_enable_stdio_uart(demo 0)
pico_enable_stdio_usb(demo 1)

target_link_libraries(demo motor pid encoder pico_stdlib)

pico_add_extra_outputs(demo)