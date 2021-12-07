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

# Vulkan option
option(VULKAN_BACKEND "option for vulkan backend" ON)
option(OPENGL_BACKEND "option for opengl backend" ON)


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

include_directories(third_party/glm)

# xml
include_directories(third_party/pugixml/src)
add_subdirectory(third_party/pugixml)

if (${VULKAN_BACKEND})
    add_definitions(-DSKITY_VULKAN=1)
    include_directories(third_party/Vulkan-Headers/include)
endif()

if (${OPENGL_BACKEND})
    add_definitions(-DSKITY_OPENGL=1)
    include_directories(third_party/OpenGL)
endif()

include_directories(include)


# freetype
include(FindFreetype)


set(Skity_header
        include/skity/codec/codec.hpp
        include/skity/codec/data.hpp
        include/skity/codec/pixmap.hpp
        include/skity/effect/path_effect.hpp
        include/skity/effect/shader.hpp
        include/skity/geometry/point.hpp
        include/skity/geometry/rect.hpp
        include/skity/geometry/rrect.hpp
        include/skity/gpu/gpu_context.hpp
        include/skity/graphic/color.hpp
        include/skity/graphic/paint.hpp
        include/skity/graphic/path.hpp
        include/skity/macros.hpp
        include/skity/render/canvas.hpp
        include/skity/skity.hpp
        include/skity/svg/svg_dom.hpp
        include/skity/text/typeface.hpp
        include/skity/text/utf.hpp
        )

# hardware canvas code
set(HW_RENDER
    src/render/hw/hw_canvas.cc
    src/render/hw/hw_canvas.hpp
    src/render/hw/hw_canvas_state.cc
    src/render/hw/hw_canvas_state.hpp
    src/render/hw/hw_draw.cc
    src/render/hw/hw_draw.hpp
    src/render/hw/hw_font_texture.hpp
    src/render/hw/hw_geometry_raster.cc
    src/render/hw/hw_geometry_raster.hpp
    src/render/hw/hw_mesh.cc
    src/render/hw/hw_mesh.hpp
    src/render/hw/hw_path_raster.cc
    src/render/hw/hw_path_raster.hpp
    src/render/hw/hw_path_visitor.cc
    src/render/hw/hw_path_visitor.hpp
    src/render/hw/hw_pipeline.hpp
    src/render/hw/hw_texture.hpp
)

# hardware Vulkan backend
if (${VULKAN_BACKEND})
    set(HW_VK_RENDER
        src/render/hw/vk/vk_canvas.cc
        src/render/hw/vk/vk_canvas.hpp
    )
else()
    set(HW_VK_RENDER "")
endif()

# hardware GL backend
if (${OPENGL_BACKEND})
    set(HW_GL_RENDER
        src/render/hw/gl/gl_canvas.cc
        src/render/hw/gl/gl_canvas.hpp
        src/render/hw/gl/gl_font_texture.cc
        src/render/hw/gl/gl_font_texture.hpp
        src/render/hw/gl/gl_interface.cc
        src/render/hw/gl/gl_interface.hpp
        src/render/hw/gl/gl_pipeline.cc
        src/render/hw/gl/gl_pipeline.hpp
        src/render/hw/gl/gl_shader.cc
        src/render/hw/gl/gl_shader.hpp
        src/render/hw/gl/gl_texture.cc
        src/render/hw/gl/gl_texture.hpp
    )
else()
   set(HW_GL_RENDER "")
endif()

set(Skity_src
        src/codec/codec.cc
        src/codec/data.cc
        src/codec/pixmap.cc
        src/effect/dash_path_effect.cc
        src/effect/dash_path_effect.hpp
        src/effect/discrete_path_effect.cc
        src/effect/discrete_path_effect.hpp
        src/effect/gradient_shader.cc
        src/effect/gradient_shader.hpp
        src/effect/path_effect.cc
        src/effect/pixmap_shader.cc
        src/effect/pixmap_shader.hpp
        src/effect/shader.cc
        src/geometry/conic.cc
        src/geometry/conic.hpp
        src/geometry/contour_measure.cc
        src/geometry/contour_measure.hpp
        src/geometry/geometry.cc
        src/geometry/geometry.hpp
        src/geometry/math.hpp
        src/geometry/point_priv.hpp
        src/geometry/rect.cc
        src/geometry/rrect.cc
        src/geometry/tesselator/mesh.cc
        src/geometry/tesselator/mesh.hpp
        src/graphic/color.cc
        src/graphic/color_parser.cc
        src/graphic/color_parser.hpp
        src/graphic/paint.cc
        src/graphic/path.cc
        src/graphic/path_measure.cc
        src/graphic/path_measure.hpp
        src/graphic/path_priv.hpp
        src/render/canvas.cc
        src/render/text/font_texture.cc
        src/render/text/font_texture.hpp
        src/render/texture_atlas.cc
        src/render/texture_atlas.hpp
        src/svg/svg_attribute.cc
        src/svg/svg_attribute.hpp
        src/svg/svg_attribute_parser.cc
        src/svg/svg_attribute_parser.hpp
        src/svg/svg_container.cc
        src/svg/svg_container.hpp
        src/svg/svg_dom.cc
        src/svg/svg_node.cc
        src/svg/svg_node.hpp
        src/svg/svg_path.cc
        src/svg/svg_path.hpp
        src/svg/svg_render_context.cc
        src/svg/svg_render_context.hpp
        src/svg/svg_root.cc
        src/svg/svg_root.hpp
        src/svg/svg_shape.cc
        src/svg/svg_shape.hpp
        src/svg/svg_transformable_node.cc
        src/svg/svg_transformable_node.hpp
        src/svg/svg_types.hpp
        src/text/typeface.cc
        src/text/utf.cc
        src/utils/lazy.hpp
        src/xml/xml_parser.cc
        src/xml/xml_parser.hpp
        ${HW_RENDER}
        ${HW_GL_RENDER}
        ${HW_VK_RENDER}
        )

set(BUILD_IN_FONT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/Roboto Mono Nerd Font Complete.ttf")
set(BUILD_IN_IMAGE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/wall.png")

set(SKITY_DEP_LIBRARIES "")

if (${FREETYPE_FOUND})
    message("Find freetype with version ${FREETYPE_VERSION_STRING}")
    include_directories(${FREETYPE_INCLUDE_DIRS})

    set(Skity_src ${Skity_src}
            src/text/ft_library_wrap.cc
            src/text/ft_library_wrap.hpp
            )

    set(ENABLE_TEXT_RENDER ON CACHE BOOL "")
    set(SKITY_DEP_LIBRARIES ${SKITY_DEP_LIBRARIES} ${FREETYPE_LIBRARIES})
else()
  message(WARNING "Freetype2 not found, text rendering will not enable")
endif()

if (WIN32)
  # libjpeg for win32
  # Fixme to solve windows can not find libjpeg
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{JPEG_PREFIX}")
endif()

# libpng
include(FindPNG)
# libjpeg-turbo
if (WIN32)
    include(FindJPEG)
else()
    find_package(PkgConfig)
    if (PkgConfig_FOUND)
        if (APPLE)
            link_directories("/usr/local/opt/jpeg-turbo/lib")
            set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/jpeg-turbo/lib/pkgconfig")
        endif()
        pkg_search_module(JPEG QUIET libturbojpeg)
    endif()
endif()

if (${PNG_FOUND})
    include_directories(${PNG_INCLUDE_DIRS})

    set(SKITY_DEP_LIBRARIES ${SKITY_DEP_LIBRARIES} ${PNG_LIBRARIES})

    add_definitions(${PNG_DEFINITIONS})
    add_definitions(-DSKITY_HAS_PNG)

    set(Skity_src ${Skity_src}
            src/codec/png_codec.hpp
            src/codec/png_codec.cc
            )
else()
  message(WARNING "libpng not found, the png file codec will not enable")
endif()

if (${JPEG_FOUND})
  include_directories(${JPEG_INCLUDE_DIRS})

  add_definitions(-DSKITY_HAS_JPEG)
  set(SKITY_DEP_LIBRARIES ${SKITY_DEP_LIBRARIES} ${JPEG_LIBRARIES})
  if (WIN32)
    # Fixme to solve windows link problem
    set(SKITY_DEP_LIBRARIES ${SKITY_DEP_LIBRARIES} "$ENV{JPEG_PREFIX}/lib/turbojpeg-static.lib")
  endif()

  set(Skity_src ${Skity_src}
      src/codec/jpeg_codec.hpp
      src/codec/jpeg_codec.cc
  )
else()
  message(WARNING "libjpeg not found, the jpg file codec will not enable")
endif()

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  shaders/hw_gl_fragment.glsl
  shaders/hw_gl_vertex.glsl
)

# source group for XCode and Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Skity_header} ${Skity_src})

#config file
configure_file(skity_config.h.in skity_config.hpp @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(skity SHARED ${Skity_header} ${Skity_src})

target_link_libraries(skity pugixml ${SKITY_DEP_LIBRARIES})

target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if (WIN32)
    set(SKITY_DLL_DIR "${CMAKE_CURRENT_BINARY_DIR}")
endif()
# test
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(third_party/gtest)
    include_directories(third_party/gtest/googletest/include)

    # Fixme to include private generate header
    add_subdirectory(test)
endif()
add_subdirectory(example)
