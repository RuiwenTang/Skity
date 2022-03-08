add_library(skity-svg STATIC)
add_library(skity::svg ALIAS skity-svg)

target_link_libraries(skity-svg PUBLIC skity)
target_link_libraries(skity-svg PUBLIC pugixml::pugixml)

target_include_directories(skity-svg PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(skity-svg PRIVATE ${SKITY_ROOT}/include)
target_include_directories(skity-svg PRIVATE ${SKITY_ROOT})

target_include_directories(
  skity-svg
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_sources(
  skity-svg
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include/skity/svg/svg_dom.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_attribute.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_attribute.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_attribute_parser.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_attribute_parser.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_color_parser.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_color_parser.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_container.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_container.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_dom.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_node.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_node.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_path.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_path.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_render_context.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_render_context.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_root.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_root.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_shape.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_shape.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_transformable_node.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_transformable_node.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/svg/svg_types.hpp
  ${CMAKE_CURRENT_LIST_DIR}/src/xml/xml_parser.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/xml/xml_parser.hpp
)

# install header
install(
  DIRECTORY
  "${CMAKE_CURRENT_LIST_DIR}/include/skity"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.hpp"
)

# install library
install(
  TARGETS skity-svg EXPORT skity-svg-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# install config file
install(
  EXPORT skity-svg-targets
  FILE skitySVGTargets.cmake
  NAMESPACE skity::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/skity
)
