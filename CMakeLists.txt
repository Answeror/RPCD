cmake_minimum_required(VERSION 2.8.5)

project(opencv)

# working directory
if(NOT WORK_DIR)
    set(WORK_DIR ${CMAKE_SOURCE_DIR}/misc/working-directory)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/misc/cmake)

# include common cmake config
include(config)

add_subdirectory(src)
