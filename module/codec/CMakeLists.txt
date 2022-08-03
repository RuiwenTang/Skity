add_library(skity-codec STATIC)
add_library(skity::codec ALIAS skity-codec)


target_include_directories(skity-codec PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(skity-codec PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(skity-codec PRIVATE ${SKITY_ROOT}/include)

target_include_directories(
  skity-codec
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_sources(
  skity-codec
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include/skity/codec/codec.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/codec/codec.cc
)


if(NOT WIN32)
  # Fixme to solve can not fild zlib
  # libpng
  add_definitions(${PNG_DEFINITIONS})
  add_definitions(-DSKITY_HAS_PNG)
  set(SKITY_HAS_PNG 1)
  target_link_libraries(skity-codec PRIVATE libpng)
endif()


# libjpeg
add_definitions(-DSKITY_HAS_JPEG)
set(SKITY_HAS_JPEG 1)

target_link_libraries(skity-codec PRIVATE turbojpeg)
target_include_directories(skity-codec PRIVATE ${CMAKE_SOURCE_DIR}/third_party/libjpeg-turbo)

# config file
configure_file(include/skity/codec/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/include/skity/codec/config.hpp)


# codec backends
if(NOT WIN32)
  target_sources(
    skity-codec
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/codec/png_codec.cc
    ${CMAKE_CURRENT_LIST_DIR}/src/codec/png_codec.hpp
  )
endif()


target_sources(
  skity-codec
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/src/codec/jpeg_codec.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/codec/jpeg_codec.hpp
)


# install header
install(
  DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/skity"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/include/skity/codec/config.hpp"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/skity/codec/
)

# install library
install(
  TARGETS skity-codec EXPORT skity-codec-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install config file
install(
  EXPORT skity-codec-targets
  FILE skityCodecTargets.cmake
  NAMESPACE skity::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_subdirectory(test)
endif()
