
if (SKITY_VCPKG)
    find_package(freetype CONFIG REQUIRED)
else()
    add_subdirectory(third_party/freetype)

    add_definitions(-DENABLE_TEXT_RENDER=1)

endif()
    
target_link_libraries(skity PRIVATE freetype)