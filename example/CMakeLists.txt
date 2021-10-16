project(skity_example)

if (NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  add_library(glad OBJECT ../third_party/glad/src/glad.c)
endif()

#GLFW
if (WIN32)
  set(glfw3_DIR "C:/Program Files (x86)/GLFW")
  set(GLFW_INCLUDE_DIRS ${glfw3_DIR}/include)
  
  find_package(OpenGL REQUIRED)
  set(GLFW_LIBRARIES ${glfw3_DIR}/lib/glfw3.lib OpenGL::GL)
  include_directories(${GLFW_INCLUDE_DIRS})
  # Fixme to solve visual studio can not find dll lib
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SKITY_DLL_DIR})
  # Fix visual studio unicode charset error
  add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
else()
find_package(PkgConfig)
  if (PkgConfig_FOUND)
    message("Pkg found")
    pkg_search_module(GLFW REQUIRED glfw3)

    link_directories("/usr/local/lib")
    include_directories(${GLFW_INCLUDE_DIRS})
  endif()
endif()

#example_config.h
set(EXAMPLE_IMAGE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/images/")
configure_file(example_config.h.in example_config.hpp @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# glad
include_directories(../third_party/glad/include)

add_executable(example example.cc $<TARGET_OBJECTS:glad>)

target_link_libraries(example skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(frame_example frame_example.cc perf.cc perf.hpp $<TARGET_OBJECTS:glad>)
target_link_libraries(frame_example skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(svg_example svg_example.cc $<TARGET_OBJECTS:glad>)
target_link_libraries(svg_example skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

if (VULKAN_BACKEND)
    add_executable(vulkan_hello_world vulkan_hello_world.cc)
    target_link_libraries(vulkan_hello_world skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})
endif()
