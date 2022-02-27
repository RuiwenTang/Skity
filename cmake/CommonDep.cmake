

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