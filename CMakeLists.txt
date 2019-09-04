cmake_minimum_required(VERSION 3.8)

project(cimgtool
        VERSION "0.0.1"
        DESCRIPTION "Compress and watermark images for web viewing"
        HOMEPAGE_URL "github.com/comfortablynick/cimgtool"
        LANGUAGES C)

# print cmake/system details
message(STATUS "CMake version:    ${CMAKE_SYSTEM_VERSION}")
message(STATUS "Compiler:         ${CMAKE_CXX_COMPILER}")
message(STATUS "Operating System: ${CMAKE_SYSTEM}")
message(STATUS "Processor:        ${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(build_safeguards)

# find dependencies
find_package(Vips REQUIRED)

# configure
message(STATUS "Running configure")
include(configure)
configure_file("${PROJECT_SOURCE_DIR}/cmake/config_h.cmake"
               "${PROJECT_SOURCE_DIR}/config.h" @ONLY)
message(STATUS "Running configure - done")

add_executable(cimgtool src/main.c)

target_include_directories(cimgtool PUBLIC ${VIPS_INCLUDE_DIRS})
target_link_libraries(cimgtool PUBLIC ${VIPS_LIBRARIES})

set_target_properties(cimgtool PROPERTIES COMPILE_OPTIONS -Wall)

# check include-what-you-use
include(iwyu)

install(TARGETS cimgtool DESTINATION $ENV{HOME}/.local/bin)
