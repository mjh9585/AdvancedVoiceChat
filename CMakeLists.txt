cmake_minimum_required(VERSION 3.18)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(AdvancedVoiceChat VERSION 0.1)

# Dependencies
add_subdirectory(libs/libdatachannel EXCLUDE_FROM_ALL)
add_subdirectory(libs/opus EXCLUDE_FROM_ALL)

# Project
add_subdirectory(src)
