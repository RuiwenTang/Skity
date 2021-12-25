if (WIN32)
  add_definitions(-DSKITY_WIN)
  set(BUILD_SHARED_LIBS TRUE)

  # specify dll outpu directory
  set(SKITY_DLL_DIR ${CMAKE_CURRENT_BINARY_DIR})
  target_compile_options(skity 
       PUBLIC
       PRIVATE
           /EHs-c- # disable exceptions
  )

else()

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
  message("build for Android")
  add_definitions(-DSKITY_ANDROID=1)
else()
  # Fixme to solve path issue
  include(cmake/CommonDep.cmake)
endif()

