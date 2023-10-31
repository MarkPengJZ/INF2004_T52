add_library(pico_ultrasonic ultrasonic.h ultrasonic.c)

target_link_libraries(pico_ultrasonic pico_stdlib hardware_gpio hardware_timer)

target_include_directories(pico_ultrasonic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


add_library(pico_ultrasonic_object ultrasonic.h ultrasonic.c ultrasonicObject.cpp ultrasonicObject.h)

target_link_libraries(pico_ultrasonic_object pico_stdlib hardware_gpio hardware_timer)

target_include_directories(pico_ultrasonic_object PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(centimeter cmExample.c)

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(centimeter pico_stdlib hardware_uart pico_ultrasonic)

pico_enable_stdio_usb(centimeter 1)
pico_enable_stdio_uart(centimeter 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(centimeter)

# add url via pico_set_program_url
example_auto_set_url(centimeter)
