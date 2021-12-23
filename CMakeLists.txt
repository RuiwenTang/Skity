cmake_minimum_required(VERSION 3.14)
project(Skity)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DSKITY_RELEASE)
endif()

set(SKITY_ROOT ${CMAKE_CURRENT_SOURCE_DIR})


# define library
add_library(skity SHARED)

# public header include
target_include_directories(skity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(skity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm)
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_library(skity::skity ALIAS skity)

# platform
include(cmake/Platform.cmake)
# options
include(cmake/Options.cmake)
# pack all shader files
include(cmake/Shader.cmake)


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
