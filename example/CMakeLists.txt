project(skity_example)

#GLFW
if (WIN32)
  set(glfw3_DIR "C:/Program Files (x86)/GLFW")
  set(GLFW_INCLUDE_DIRS ${glfw3_DIR}/include)
  
  find_package(OpenGL REQUIRED)
  set(GLFW_LIBRARIES ${glfw3_DIR}/lib/glfw3.lib OpenGL::GL)
  include_directories(${GLFW_INCLUDE_DIRS})
else()
find_package(PkgConfig)
  if (PkgConfig_FOUND)
    message("Pkg found")
    pkg_search_module(GLFW REQUIRED glfw3)

    link_directories("/usr/local/lib")
    include_directories(${GLFW_INCLUDE_DIRS})
  endif()
endif()

# glad
include_directories(../third_party/glad/include)

add_executable(example example.cc $<TARGET_OBJECTS:glad>)

target_link_libraries(example skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})

add_executable(frame_example frame_example.cc perf.cc perf.hpp $<TARGET_OBJECTS:glad>)
target_link_libraries(frame_example skity ${GLFW_LIBRARIES} ${CMAKE_DL_LIBS})
