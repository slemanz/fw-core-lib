cmake_minimum_required(VERSION 3.21)
project(FwCoreLib VERSION 0.1.0 LANGUAGES C CXX)

set(COMMON_SOURCES_CORE
    Src/core/cli.c
    Src/core/fault.c
    Src/core/simple-timer.c
    Src/core/ticker.c
    Src/core/uprint.c
)

set(COMMON_SOURCES_BSP
    Src/bsp/button.c
    Src/bsp/led.c
    Src/bsp/output.c
    Src/bsp/rtc.c
)

set(COMMON_SOURCES_SHARED
    Src/shared/pool.c
    Src/shared/ring-buffer.c
    Src/shared/slist.c
)

# Agrupa todos os sources em uma variável só para o add_library
set(COMMON_ALL_SOURCES
    ${COMMON_SOURCES_CORE}
    ${COMMON_SOURCES_BSP}
    ${COMMON_SOURCES_SHARED}
)

add_library(fw_core_lib STATIC ${COMMON_ALL_SOURCES})

target_include_directories(fw_core_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Inc
)

target_compile_options(fw_core_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter 
)