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

# set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
# include(compiler_check)
include(build_safeguards)

# find dependencies find_package(fmt) find_package(loguru)
find_package(Vips REQUIRED)

set(SOURCE_FILES src/main.c)

# configure message(STATUS "Running configure") include(configure)
# configure_file("${PROJECT_SOURCE_DIR}/cmake/config_h.cmake"
# "${PROJECT_SOURCE_DIR}/config.h" @ONLY) message(STATUS "Running configure -
# done")

add_executable(cimgtool ${SOURCE_FILES})

target_include_directories(cimgtool PUBLIC ${VIPS_INCLUDE_DIRS})
target_link_libraries(cimgtool PUBLIC ${VIPS_LIBRARIES})

# set_target_properties(cimgtool PROPERTIES COMPILE_OPTIONS -pedantic -Wall
# -Wextra)
set_target_properties(cimgtool PROPERTIES COMPILE_OPTIONS -Wall)
# -Wall `pkg-config vips --cflags --libs`)

# check include-what-you-use
include(iwyu)

# install install(TARGETS cimgtool DESTINATION $ENV{HOME}/.local/bin)
install(TARGETS cimgtool DESTINATION ${PROJECT_SOURCE_DIR})
