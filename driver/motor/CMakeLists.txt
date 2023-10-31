add_library(motor motor.h motor.c)
add_library(pid pid.h pid.c)

target_link_libraries(motor pico_stdlib hardware_pwm encoder)
