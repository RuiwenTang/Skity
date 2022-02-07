
target_include_directories(
  skity
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(SKITY_PUBLIC_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/skity/codec/codec.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/codec/data.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/codec/pixmap.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/effect/mask_filter.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/effect/path_effect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/effect/shader.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/point.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/rect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/rrect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/gpu/gpu_context.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/gpu/gpu_vk_context.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/color.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/paint.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/path.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/macros.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/render/canvas.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/skity.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/text/text_blob.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/text/text_run.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/text/typeface.hpp
    ${CMAKE_CURRENT_LIST_DIR}/skity/text/utf.hpp
)

target_sources(
  skity
  PRIVATE
  ${SKITY_PUBLIC_HEADERS}
)

# config file
configure_file(skity/config.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/skity/config.hpp)

# install header
install(DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/skity"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/skity/config.hpp"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/skity
)