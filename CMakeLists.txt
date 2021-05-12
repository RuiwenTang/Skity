cmake_minimum_required(VERSION 3.16)
project(Skity)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output} "#pragma once\n")
    # Collect input files
    file(GLOB bins ${dir}/*)
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
include_directories(third_party/glad/include)
include_directories(third_party/glm)

include_directories(include)

add_library(glad OBJECT third_party/glad/src/glad.c)

set(Skity_header
        include/skity/geometry/point.hpp
        include/skity/geometry/rect.hpp
        include/skity/graphic/paint.hpp
        include/skity/graphic/path.hpp
        include/skity/render/canvas.hpp
        )

set(Skity_src
        src/geometry/conic.cc
        src/geometry/conic.hpp
        src/geometry/geometry.cc
        src/geometry/geometry.hpp
        src/geometry/math.hpp
        src/geometry/point_priv.hpp
        src/geometry/quad_construct.cc
        src/geometry/quad_construct.hpp
        src/geometry/rect.cc
        src/graphic/paint.cc
        src/graphic/path.cc
        src/render/canvas.cc
        src/render/cap_proc.cc
        src/render/cap_proc.hpp
        src/render/draw.cc
        src/render/draw.hpp
        src/render/join_proc.cc
        src/render/join_proc.hpp
        src/render/path_raster.cc
        src/render/path_raster.hpp
        src/render/path_stroker.cc
        src/render/path_stroker.hpp
        src/render/path_vertex.cc
        src/render/path_vertex.hpp
        src/render/stroke.cc
        src/render/stroke.hpp
        )

add_library(skity ${Skity_header} ${Skity_src} $<TARGET_OBJECTS:glad>)

target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# test
add_subdirectory(third_party/gtest)
include_directories(third_party/gtest/googletest/include)

add_subdirectory(test)
