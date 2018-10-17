cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Framework LANGUAGES CXX)

include_directories("${LoggerUtils_SOURCE_DIR}/include")
add_definitions("-DACSDK_LOG_MODULE=frameware")

add_subdirectory("DeviceIo")

if(${Platform} STREQUAL Hodor
        OR ${Platform} STREQUAL MTK)
    add_subdirectory("Timer")
endif()
