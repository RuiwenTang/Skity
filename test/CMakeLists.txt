
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

add_executable(math_test math_test.cc)
target_link_libraries(math_test gtest skity)

add_executable(geometry_test geometry_test.cc)
target_link_libraries(geometry_test gtest skity)

add_executable(path_test path_test.cc)
target_link_libraries(path_test gtest skity)

add_executable(stroke_test stroke_test.cc)
target_link_libraries(stroke_test gtest skity)

add_executable(path_raster_test path_raster_test.cc)
target_link_libraries(path_raster_test gtest skity)

#GLFW
find_package(PkgConfig)
if (PkgConfig_FOUND)
  message("Pkg found")

  add_library(test_common
    OBJECT
    common/test_common.cc
    common/test_common.hpp
  )

  pkg_search_module(GLFW REQUIRED glfw3)

  link_directories("/usr/local/lib")
  include_directories(${GLFW_INCLUDE_DIRS})
  
  add_executable(raw_gl_render_test raw_gl_render_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(raw_gl_render_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

  add_executable(ms_paper_test ms_paper_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(ms_paper_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

  add_executable(cubic_fill_stencil_test cubic_fill_stencil_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(cubic_fill_stencil_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

  add_executable(gl_path_mesh_test gl_path_mesh_test.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(gl_path_mesh_test skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

  add_executable(gl_path_stroke_mesh gl_path_stroke_mesh.cc $<TARGET_OBJECTS:test_common> $<TARGET_OBJECTS:glad>)
  target_link_libraries(gl_path_stroke_mesh skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

  message(${FREETYPE_INCLUDE_DIRS})
  add_executable(freetype_path_test freetype_path_test.cc)
  target_include_directories(freetype_path_test PRIVATE ${FREETYPE_INCLUDE_DIRS})
  target_link_libraries(freetype_path_test ${FREETYPE_LIBRARIES})
endif()
