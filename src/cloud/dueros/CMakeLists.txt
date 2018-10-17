cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set project information
project(DCSDeviceApp VERSION 1.0.3 LANGUAGES CXX)

ADD_COMPILE_OPTIONS(-g -Wall -Os -w -fPIC -s -std=c++11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Os -w -fPIC -s")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__LOG_FILENAME__='\"$(notdir $<)\"' -D__SAMPLEAPP_VERSION__='\"${__SAMPLEAPP_VERSION__}\"' -D__DCSSDK_VERSION__='\"${__DCSSDK_VERSION__}\"' -D__DUER_LINK_VERSION__='\"${__DUER_LINK_VERSION__}\"'")

include_directories(
    sdk/include
    sdk/include/duer_link
    sdk/include/duer_link/network_observer
    sdk/include/duer_link/device_ctrl_observer
    third/include
    Framework/include)
link_directories(sdk/${CMAKE_INSTALL_LIBDIR})

add_definitions(-DLinux)

add_subdirectory("DCSApp")
add_subdirectory("Framework")

file(GLOB duer_sdk_lib "sdk/${CMAKE_INSTALL_LIBDIR}/*.so")
install(FILES ${duer_sdk_lib} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
