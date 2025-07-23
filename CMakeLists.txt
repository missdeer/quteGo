cmake_minimum_required(VERSION 3.15)

# Set policy for Boost to avoid deprecation warning (affects all subdirectories)
cmake_policy(SET CMP0167 NEW)

project(quteGoProject VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Always set the architectures for macOS
if(APPLE AND NOT IOS)
#  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64;x86_64h" CACHE STRING "Build architectures for OSX" FORCE)
endif()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_TYPE Debug)
    add_compile_definitions(LOGS_ENABLED)
ELSE(CMAKE_BUILD_TYPE MATCHES Debug)
    set(BUILD_TYPE Release)
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

option(AVX2 "Target CPU does support AVX2")

set(CURRENT_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
message("Compiler ID is: ${CURRENT_COMPILER_ID}")

set(CURRENT_SIMULATE_ID "${CMAKE_CXX_SIMULATE_ID}")
message("Simulate ID is: ${CURRENT_SIMULATE_ID}")

include(cmake/avx.cmake)

if(WIN32)
    add_compile_definitions(STRSAFE_NO_DEPRECATE UNICODE _UNICODE _USE_MATH_DEFINES)
endif()

if (MSVC)
    add_compile_options(/EHsc)
    add_compile_options(/utf-8)
endif()

add_subdirectory(thirdparty)
add_subdirectory(src)
