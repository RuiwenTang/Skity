
target_include_directories(
  skity
  PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(
  skity
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/skity/codec/codec.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/codec/data.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/codec/pixmap.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/effect/path_effect.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/effect/shader.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/point.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/rect.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/geometry/rrect.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/gpu/gpu_context.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/color.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/paint.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/graphic/path.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/macros.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/render/canvas.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/skity.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/text/typeface.hpp
  ${CMAKE_CURRENT_LIST_DIR}/skity/text/utf.hpp
)