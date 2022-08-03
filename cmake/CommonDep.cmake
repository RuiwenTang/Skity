
add_subdirectory(third_party/freetype)

add_definitions(-DENABLE_TEXT_RENDER=1)

target_link_libraries(skity PRIVATE freetype)
