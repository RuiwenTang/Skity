cmake_minimum_required(VERSION 3.14)
project(Skity)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (WIN32)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
        set(BUILD_SHARED_LIBS TRUE)
    endif()
else()
    add_definitions(-DSKITY_RELEASE)
    if (WIN32)
        add_definitions(-DSKITY_WIN)
    else()
        set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-o2")
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-o2")
    endif()
endif()

set(SKITY_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

option(BUILD_EXAMPLE "option for building example" ON)
option(BUILD_TEST "option for building test" ON)
option(BUILD_SVG_MODULE "option for build svg module" ON)
# logging option
option(ENABLE_LOG "option for logging" ON)
# backend option
option(ENABLE_HW_RENDER "option for hardware backends" ON)
option(VULKAN_BACKEND "option for vulkan backend" OFF)
option(OPENGL_BACKEND "option for opengl backend" ON)


# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "#pragma once\n")
    # Collect input files
    file(GLOB bins ${dir}/*.glsl ${dir}/*.spv)
    message(${bins})
    # Iterate through input files
    foreach (bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
    endforeach ()
endfunction()

create_resources(${CMAKE_CURRENT_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shader.hpp)

#include
include_directories(third_party/glm)

if (${ENABLE_LOG})
    add_definitions(-DSKITY_LOG=1)
    # no exception
    add_definitions(-DSPDLOG_NO_EXCEPTIONS)
    include_directories(third_party/spdlog/include)
endif()


if (${VULKAN_BACKEND})
    add_definitions(-DSKITY_VULKAN=1)
    include_directories(third_party/Vulkan-Headers/include)
    include_directories(third_party/VulkanMemoryAllocator/include)
endif()

if (${OPENGL_BACKEND})
    add_definitions(-DSKITY_OPENGL=1)
    include_directories(third_party/OpenGL)
endif()

add_library(skity SHARED)

# public header include
target_include_directories(skity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_library(skity::skity ALIAS skity)

# svg module
if (${BUILD_SVG_MODULE})
    add_subdirectory(third_party/pugixml)
    add_subdirectory(module/svg)
endif()

if (WIN32)
   # specify dll outpu directory
   set(SKITY_DLL_DIR ${CMAKE_CURRENT_BINARY_DIR})
   target_compile_options(skity 
        PUBLIC
        PRIVATE
            /EHs-c- # disable exceptions
   )
else()

    target_compile_options(skity 
        PUBLIC
        PRIVATE
            -fno-exceptions
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(skity 
            PUBLIC
            PRIVATE
            -fvisibility=hidden
        )
    endif()
endif()


# dependencies
if (WIN32)
    # libjpeg for win32
    # Fixme to solve windows can not find libjpeg
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{JPEG_PREFIX}")
endif()

include(FindPNG)

if (WIN32)
    inlcude(FindJPEG)
else()
    find_package(PkgConfig)
    if (PkgConfig_FOUND)
        if (APPLE)
            target_link_directories(skity PRIVATE "/usr/local/opt/jpeg-turbo/lib")
            set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/jpeg-turbo/lib/pkgconfig")
        endif()
        pkg_search_module(JPEG QUIET libturbojpeg)
    endif()
endif()

# libpng
if (${PNG_FOUND})
    target_include_directories(skity PRIVATE ${PNG_INCLUDE_DIRS})

    add_definitions(${PNG_DEFINITIONS})
    add_definitions(-DSKITY_HAS_PNG)

    target_link_libraries(skity PRIVATE ${PNG_LIBRARIES})
else()
    message(WARNING "libpng not found, the png file codec will not enable")
endif()

# libjpeg
if (${JPEG_FOUND})
    target_include_directories(skity PRIVATE ${JPEG_INCLUDE_DIRS})

    add_definitions(-DSKITY_HAS_JPEG)
    target_link_libraries(skity PRIVATE ${JPEG_LIBRARIES})
else()
    message(WARNING "libjpeg not found, the jpg file codec will not enable")
endif()

# Freetype
include(FindFreetype)
if (${FREETYPE_FOUND})
    message("Find freetype with version ${FREETYPE_VERSION_STRING}")

    target_include_directories(skity PRIVATE ${FREETYPE_INCLUDE_DIRS})

    add_definitions(-DENABLE_TEXT_RENDER=1)
    target_link_libraries(skity PRIVATE ${FREETYPE_LIBRARIES})
else()
    message(WARNING "Freetype2 not found, text rendering will not enable")
endif()

# header files
add_subdirectory(include)
# source files
add_subdirectory(src)

# source group for Xcode and Visual Studio
get_target_property(SKITY_SRC skity SOURCES)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SKITY_SRC})


if (${BUILD_TEST} AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(third_party/gtest)
    add_subdirectory(test)
endif()

if (${BUILD_EXAMPLE})
    add_subdirectory(example)
endif()
