cmake_minimum_required(VERSION 3.10)
project(DSP_Overdrive VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_CXX_VISIBILITY_PRESET default)
set(CMAKE_C_VISIBILITY_PRESET default)

set(DSP_SOURCES
    Core/Effects/dsp.cpp
)

set(DSP_HEADERS
    Core/Effects/dsp.hpp
)

add_library(dsp_overdrive SHARED ${DSP_SOURCES})

set_target_properties(dsp_overdrive PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${DSP_HEADERS}"
    POSITION_INDEPENDENT_CODE ON
    PREFIX "lib"   
)

target_link_libraries(dsp_overdrive PRIVATE m)

target_include_directories(dsp_overdrive PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core/Effects>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS dsp_overdrive
    LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)

add_custom_command(TARGET dsp_overdrive POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:dsp_overdrive>
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/
)

add_custom_command(TARGET dsp_overdrive POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "====================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Library built successfully!"
    COMMAND ${CMAKE_COMMAND} -E echo "Location: $<TARGET_FILE:dsp_overdrive>"
    COMMAND ${CMAKE_COMMAND} -E echo "====================================="
)

