# Install script for directory: /Users/elias/Documents/GitHub/embeddedprj/robot_car

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/opt/homebrew/bin/arm-none-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/encoder/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/motor/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/ultrasonic/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/irline_navigation/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/irline_barcode/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/line_detector/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/magnetometer/cmake_install.cmake")
  include("/Users/elias/Documents/GitHub/embeddedprj/build/robot_car/pico_w_webserver/cmake_install.cmake")

endif()

