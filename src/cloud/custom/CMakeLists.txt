cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(UICustomized VERSION 0.0.1 LANGUAGES C)
set(PROJECT_BRIEF "Customized user interactions for the clouds")

# add_subdirectory("led")
INCLUDE_DIRECTORIES(./)
INCLUDE_DIRECTORIES(./led)
INCLUDE_DIRECTORIES(./led/inc)

# add_library(uicustom SHARED $<TARGET_OBJECTS:LED>)
add_library(uicustom SHARED ./led/led_app.c ./led/fl3236_led.c)

install(FILES libuicustom.so DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES led/inc/led_app.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})