if (WIN32)
  add_definitions(-DSKITY_WIN)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    set(BUILD_SHARED_LIBS TRUE)
  endif()

  # specify dll outpu directory
  set(SKITY_DLL_DIR ${CMAKE_CURRENT_BINARY_DIR})
  target_compile_options(skity 
       PUBLIC
       PRIVATE
           /EHs-c- # disable exceptions
  )

else()
  set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-o2")
  set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-o2")

  target_compile_options(skity 
        PUBLIC
        PRIVATE
            -fno-exceptions
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(skity 
            PUBLIC
            PRIVATE
            -fvisibility=hidden
        )
    endif()

endif()


# dependencies
if (ANDROID)
  # TODO support android build
else()
  # Fixme to solve path issue
  include(cmake/CommonDep.cmake)
endif()

