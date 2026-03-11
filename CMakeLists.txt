cmake_minimum_required(VERSION 3.20.0)

project(badges_for_players VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode! Please set the GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE SPRITE_FILES CONFIGURE_DEPENDS "resources/*.png")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${SPRITE_FILES})

setup_geode_mod(${PROJECT_NAME})
