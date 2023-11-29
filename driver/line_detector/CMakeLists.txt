add_library(line_detector line_detector.h line_detector.c)

target_link_libraries(line_detector pico_stdlib hardware_adc)
