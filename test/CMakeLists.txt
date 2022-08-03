set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

#config file
set(TEST_BUILD_IN_FONT "${CMAKE_CURRENT_SOURCE_DIR}/../resources/Avenir.ttf")
set(TEST_BUILD_IN_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../resources/wall.png")
set(TEST_BUILD_IN_EMOJI_FONT "${CMAKE_CURRENT_SOURCE_DIR}/../example/images/NotoEmoji-Regular.ttf")
configure_file(test_config.h.in test_config.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

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

add_executable(textblob_test textblob_test.cc)
target_link_libraries(textblob_test gtest skity)

message("test cmake")

add_library(
  test_common
  OBJECT
  common/test_common.cc
  common/test_common.hpp
)

target_link_libraries(test_common glfw)

add_executable(ms_paper_test ms_paper_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(ms_paper_test skity glfw ${CMAKE_DL_LIBS})

add_executable(hw_canvas_test hw_canvas_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(hw_canvas_test skity skity-codec glfw ${CMAKE_DL_LIBS})

add_executable(fill_rule_test fill_rule_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(fill_rule_test skity glfw ${CMAKE_DL_LIBS})

add_executable(raw_gl_tessellation_test raw_gl_tessellation_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(raw_gl_tessellation_test skity glfw ${CMAKE_DL_LIBS})

add_executable(raw_gl_geometry_test raw_gl_geometry_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
target_link_libraries(raw_gl_geometry_test skity glfw ${CMAKE_DL_LIBS})

add_executable(svg_parser_test svg_parser_test.cc)
target_include_directories(svg_parser_test PRIVATE ${SKITY_ROOT}/module/svg/include)
target_link_libraries(svg_parser_test skity skity::svg)

add_executable(texture_atlas_test texture_atlas_test.cc)
target_link_libraries(texture_atlas_test skity)

add_executable(font_texture_test font_texture_test.cc)
target_link_libraries(font_texture_test skity)

add_executable(freetype_path_test freetype_path_test.cc)
target_link_libraries(freetype_path_test freetype)

if (${CPU_BACKEND})
  add_executable(sw_canvas_test sw_canvas_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(sw_canvas_test skity skity-codec glfw ${CMAKE_DL_LIBS})

endif()
