cmake_minimum_required(VERSION 3.10.0)

project(Skity)

set(SKITY_VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-DSKITY_RELEASE)
endif()

set(SKITY_ROOT ${CMAKE_CURRENT_SOURCE_DIR})


# define library
if (EMSCRIPTEN)
  add_library(skity STATIC)
else()
  add_library(skity SHARED)
endif()

# public header include
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(skity PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(
  skity PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

add_library(skity::skity ALIAS skity)

# default config
include(GNUInstallDirs)
# platform
include(cmake/Platform.cmake)
# options
include(cmake/Options.cmake)
# pack all shader files
include(cmake/Shader.cmake)
# third party deps
include(cmake/ThirdPartyDep.cmake)

if(${ENABLE_LOG})
  set(SKITY_LOG 1)
  # no exception
  add_definitions(-DSPDLOG_NO_EXCEPTIONS)
endif()


if(${VULKAN_BACKEND})
  set(SKITY_VULKAN 1)
endif()

if(${OPENGL_BACKEND})
  set(SKITY_OPENGL 1)
endif()

if(${CPU_BACKEND})
  set(SKITY_CPU 1)
endif()

# svg module
if(${BUILD_SVG_MODULE})
  add_subdirectory(module/svg)
endif()

if(${BUILD_CODEC_MODULE})
  add_subdirectory(third_party/libpng)
  add_subdirectory(third_party/libjpeg-turbo)
  add_subdirectory(module/codec)
endif()


# header files
add_subdirectory(include)
# source files
add_subdirectory(src)

# source group for Xcode and Visual Studio
get_target_property(SKITY_SRC skity SOURCES)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SKITY_SRC})

if (${BUILD_TEST} OR ${BUILD_EXAMPLE})
  add_subdirectory(third_party/glfw)
endif()

if(${BUILD_TEST} AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_subdirectory(third_party/gtest)
  add_subdirectory(test)
endif()

if(${BUILD_EXAMPLE})
  add_subdirectory(example)
endif()

# package config file
include(CMakePackageConfigHelpers)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/skityConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)

# version
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/skityConfigVersion.cmake
  VERSION ${SKITY_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/skityConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/skityConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)

# install library
install(
  TARGETS skity EXPORT skity-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install config file
install(
  EXPORT skity-targets
  FILE skityTargets.cmake
  NAMESPACE skity::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)
