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

# freetype
find_package(Freetype REQUIRED)

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
        src/geometry/rect.cc
        src/graphic/paint.cc
        src/graphic/path.cc
        src/render/canvas.cc
        src/render/gl/draw/gl_draw_mesh_op.cc
        src/render/gl/draw/gl_draw_mesh_op.hpp
        src/render/gl/draw/gl_fill_color_op.cc
        src/render/gl/draw/gl_fill_color_op.hpp
        src/render/gl/draw/gl_stencil_op.cc
        src/render/gl/draw/gl_stencil_op.hpp
        src/render/gl/gl_canvas.cc
        src/render/gl/gl_canvas.hpp
        src/render/gl/gl_draw_op.cc
        src/render/gl/gl_draw_op.hpp
        src/render/gl/gl_fill.cc
        src/render/gl/gl_fill.hpp
        src/render/gl/gl_mesh.cc
        src/render/gl/gl_mesh.hpp
        src/render/gl/gl_path_visitor.cc
        src/render/gl/gl_path_visitor.hpp
        src/render/gl/gl_shader.cc
        src/render/gl/gl_shader.hpp
        src/render/gl/gl_stroke.cc
        src/render/gl/gl_stroke.hpp
        src/render/gl/gl_vertex.cc
        src/render/gl/gl_vertex.hpp
        )

add_library(skity ${Skity_header} ${Skity_src})

target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# test
add_subdirectory(third_party/gtest)
include_directories(third_party/gtest/googletest/include)

# Fixme to include private generate header
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_subdirectory(test)
