

if (WIN32)
  # libjpeg for win32
  # Fixme to solve windows can not find libjpeg
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "$ENV{JPEG_PREFIX}")
endif()

include(FindPNG)

if (WIN32)
  include(FindJPEG)
else()
  find_package(PkgConfig)
  if (PkgConfig_FOUND)
      if (APPLE)
          target_link_directories(skity PRIVATE "/usr/local/opt/jpeg-turbo/lib")
          set(ENV{PKG_CONFIG_PATH} "/usr/local/opt/jpeg-turbo/lib/pkgconfig")
      endif()
      pkg_search_module(JPEG QUIET libturbojpeg)
  endif()
endif()

# libpng
if (${PNG_FOUND})
  target_include_directories(skity PRIVATE ${PNG_INCLUDE_DIRS})

  add_definitions(${PNG_DEFINITIONS})
  add_definitions(-DSKITY_HAS_PNG)

  target_link_libraries(skity PRIVATE ${PNG_LIBRARIES})
else()
  message(WARNING "libpng not found, the png file codec will not enable")
endif()

# libjpeg
if (${JPEG_FOUND})
  target_include_directories(skity PRIVATE ${JPEG_INCLUDE_DIRS})

  add_definitions(-DSKITY_HAS_JPEG)
  target_link_libraries(skity PRIVATE ${JPEG_LIBRARIES})
  if (WIN32)
    # Fixme to solve windows link problem
    target_link_libraries(skity PRIVATE "$ENV{JPEG_PREFIX}/lib/turbojpeg-static.lib")
  endif()
else()
  message(WARNING "libjpeg not found, the jpg file codec will not enable")
endif()


# Freetype
include(FindFreetype)
if (${FREETYPE_FOUND})
    message("Find freetype with version ${FREETYPE_VERSION_STRING}")

    target_include_directories(skity PRIVATE ${FREETYPE_INCLUDE_DIRS})

    add_definitions(-DENABLE_TEXT_RENDER=1)
    target_link_libraries(skity PRIVATE ${FREETYPE_LIBRARIES})
else()
    message(WARNING "Freetype2 not found, text rendering will not enable")
endif()