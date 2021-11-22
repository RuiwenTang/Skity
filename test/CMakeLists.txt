
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)
# glad
include_directories(../third_party/glad/include)
add_library(glad OBJECT ../third_party/glad/src/glad.c)

add_executable(math_test math_test.cc)
target_link_libraries(math_test gtest skity)

add_executable(geometry_test geometry_test.cc)
target_link_libraries(geometry_test gtest skity)

add_executable(path_test path_test.cc)
target_link_libraries(path_test gtest skity)

add_executable(path_measure_test path_measure_test.cc)
target_link_libraries(path_measure_test gtest skity)

#GLFW
if (WIN32)
    set(glfw3_DIR "C:/Program Files (x86)/GLFW")
    set(GLFW_INCLUDE_DIRS ${glfw3_DIR}/include)

    find_package(OpenGL REQUIRED)
    set(GLFW_LIBRARIES ${glfw3_DIR}/lib/glfw3.lib OpenGL::GL)
    include_directories(${GLFW_INCLUDE_DIRS})
      # Fixme to solve visual studio can not find dll lib
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SKITY_DLL_DIR})
else ()
    find_package(PkgConfig)
    if (PkgConfig_FOUND)
        message("Pkg found")
        pkg_search_module(GLFW REQUIRED glfw3)

        link_directories("/usr/local/lib")
        include_directories(${GLFW_INCLUDE_DIRS})
    endif ()
endif ()

add_library(test_common
        OBJECT
        common/test_common.cc
        common/test_common.hpp
        )
add_executable(raw_gl_render_test raw_gl_render_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(raw_gl_render_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(ms_paper_test ms_paper_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(ms_paper_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(cubic_fill_stencil_test cubic_fill_stencil_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(cubic_fill_stencil_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(gl_canvas_test gl_canvas_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(gl_canvas_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(gl_canvas2_test gl_canvas2_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(gl_canvas2_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(canvas_aa_test canvas_aa_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(canvas_aa_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(shader_aa_test shader_aa_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(shader_aa_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(hw_canvas_test hw_canvas_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(hw_canvas_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(svg_parser_test svg_parser_test.cc)
target_link_libraries(svg_parser_test skity)

if (${FREETYPE_FOUND})
    message(${FREETYPE_INCLUDE_DIRS})
    add_executable(freetype_path_test freetype_path_test.cc)
    target_include_directories(freetype_path_test PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(freetype_path_test ${FREETYPE_LIBRARIES})
endif ()

if (${PNG_FOUND})
    add_executable(png_decode_test png_decode_test.cc)
    target_link_libraries(png_decode_test skity ${PNG_LIBRARIES})
endif ()