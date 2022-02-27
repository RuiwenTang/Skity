add_library(skity-codec STATIC)
add_library(skity::codec ALIAS skity-codec)


target_include_directories(skity-codec PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(skity-codec PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(skity-codec PRIVATE ${SKITY_ROOT}/include)

target_include_directories(skity-codec
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_sources(skity-codec
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include/skity/codec/codec.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/codec/codec.cc
)


if (WIN32)
  # libjpeg for win32
  # Fixme to solve windows can not find libjpeg
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{JPEG_PREFIX}")
endif()

# include(FindPNG)
find_package(PNG 1.6.0)

if (WIN32)
  include(FindJPEG)
else()
  find_package(PkgConfig)
  if (PkgConfig_FOUND)
      if (APPLE)
          target_link_directories(skity-codec PRIVATE "/usr/local/opt/jpeg-turbo/lib")
          set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/jpeg-turbo/lib/pkgconfig")
      endif()
      pkg_search_module(JPEG QUIET libturbojpeg)
  endif()
endif()

# libpng
if (${PNG_FOUND})
  target_include_directories(skity-codec PRIVATE ${PNG_INCLUDE_DIRS})

  add_definitions(${PNG_DEFINITIONS})
  add_definitions(-DSKITY_HAS_PNG)

  target_link_libraries(skity-codec PRIVATE ${PNG_LIBRARIES})
else()
  message(WARNING "libpng not found, the png file codec will not enable")
endif()

# libjpeg
if (${JPEG_FOUND})
  target_include_directories(skity-codec PRIVATE ${JPEG_INCLUDE_DIRS})

  add_definitions(-DSKITY_HAS_JPEG)
  target_link_libraries(skity-codec PRIVATE ${JPEG_LIBRARIES})
  if (WIN32)
    # Fixme to solve windows link problem
    target_link_libraries(skity-codec PRIVATE "$ENV{JPEG_PREFIX}/lib/turbojpeg-static.lib")
  endif()
else()
  message(WARNING "libjpeg not found, the jpg file codec will not enable")
endif()

# codec backends
if (${PNG_FOUND})
  target_sources(skity-codec
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/src/codec/png_codec.cc
      ${CMAKE_CURRENT_LIST_DIR}/src/codec/png_codec.hpp
  )
endif()

if (${JPEG_FOUND})
  target_sources(skity-codec
    PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/src/codec/jpeg_codec.cc
      ${CMAKE_CURRENT_LIST_DIR}/src/codec/jpeg_codec.hpp
  )
endif()

# install header
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/skity"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

# install library
install(TARGETS skity-codec EXPORT skity-codec-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install config file
install(EXPORT skity-codec-targets
  FILE        skityCodecTargets.cmake
  NAMESPACE   skity::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)
