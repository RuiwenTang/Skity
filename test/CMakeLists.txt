
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

  pkg_search_module(GLFW REQUIRED glfw3)

  link_directories("/usr/local/lib")
  include_directories(${GLFW_INCLUDE_DIRS})
  
  add_executable(raw_gl_render_test raw_gl_render_test.cc)
  target_link_libraries(raw_gl_render_test skity ${GLFW_LIBRARIES})
endif()
