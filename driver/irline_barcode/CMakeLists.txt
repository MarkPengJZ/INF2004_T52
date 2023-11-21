add_executable(infraredMain
            infraredMain.c
        )

# pull in common dependencies
target_link_libraries(infraredMain pico_stdlib)

# enable PUTTY console debugging :D
pico_enable_stdio_usb(infraredMain 1)
pico_enable_stdio_uart(infraredMain 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(infraredMain)
