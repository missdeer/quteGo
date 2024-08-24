cmake_minimum_required(VERSION 3.15)
project(quteGoProject VERSION 1.0.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
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

if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "amd64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        if(NOT "${CURRENT_COMPILER_ID}"  STREQUAL "MSVC")
            message("not MSVC")
            add_compile_options("-mpopcnt" "-mbmi" "-mbmi2" "-mlzcnt" "-mmovbe" "-mfma")
        endif()
        if(AVX2)
            if("${CURRENT_COMPILER_ID}" STREQUAL "IntelLLVM")
                message("Intel oneAPI enables AVX2")
                add_compile_options("/QxCORE-AVX2" "-mavx2")
            elseif ("${CURRENT_COMPILER_ID}" STREQUAL "MSVC" OR "${CURRENT_SIMULATE_ID}" STREQUAL "MSVC")
                message("MSVC or Clang-Cl enables AVX2")
                add_compile_options("/Ot" "/fp:fast" "/arch:AVX2")
            else()
                message("Others eg GCC/Clang... enables AVX2")
                add_compile_options("-mavx2")
            endif()
        endif()
    endif()
endif()

if(WIN32)
    add_compile_definitions(STRSAFE_NO_DEPRECATE UNICODE _UNICODE _USE_MATH_DEFINES)
endif()

if (MSVC)
    if("${CURRENT_COMPILER_ID}" STREQUAL "IntelLLVM")
        add_compile_options(/EHsc)
    else()
        add_compile_options(/utf-8)
    endif()
endif()

add_subdirectory(src)

if (WIN32)
set(QT7z_NO_TEST ON)
add_subdirectory(src/thirdparty/Qt7z)

set(QTRAR_NO_TEST ON)
add_subdirectory(src/thirdparty/QtRAR)

target_compile_definitions(qt7z PUBLIC QT7Z_STATIC 
    QT7Z_STATIC_LINK 
    WIN_LONG_PATH 
    _7ZIP_LARGE_PAGES 
    SUPPORT_DEVICE_FILE
    )
target_compile_definitions(qtrar PUBLIC QTRAR_STATIC)
endif()