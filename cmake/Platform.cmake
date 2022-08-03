if (EMSCRIPTEN)
  # Build Skity with emsdk
  add_definitions(-DSKITY_WASM)
elseif(WIN32)
  add_definitions(-DSKITY_WIN)
  set(BUILD_SHARED_LIBS TRUE)

  # specify dll outpu directory
  set(SKITY_DLL_DIR ${CMAKE_CURRENT_BINARY_DIR})
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SKITY_DLL_DIR})

  target_compile_options(
    skity
    PUBLIC
    PRIVATE
    /EHs-c- # disable exceptions
  )

else()

  target_compile_options(
    skity
    PUBLIC
    PRIVATE
    -fno-exceptions
  )

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(
      skity
      PUBLIC
      PRIVATE
      -fvisibility=hidden
    )
  endif()

endif()

# dependencies
if (EMSCRIPTEN)
  # use emsd port freetype
  add_definitions(-DENABLE_TEXT_RENDER=1)
  # use emscripten port freetype
  set(FREETYPE_FOUND True)

  target_compile_options(skity PUBLIC "-s" "USE_FREETYPE=1")
  set_target_properties(skity PROPERTIES LINK_FLAGS "-s USE_FREETYPE=1")
else()
  # Fixme to solve path issue
  include(cmake/CommonDep.cmake)
endif()
