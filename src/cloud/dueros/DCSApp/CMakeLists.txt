cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(DCSApp LANGUAGES CXX)

include(../build/BuildDefaults.cmake)
add_definitions("-DACSDK_LOG_MODULE=duer_linux")

add_subdirectory("src")
