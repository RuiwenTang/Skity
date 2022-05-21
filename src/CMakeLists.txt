target_sources(
  skity
  PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/io/data.cc
  ${CMAKE_CURRENT_LIST_DIR}/io/pixmap.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/dash_path_effect.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/dash_path_effect.hpp
  ${CMAKE_CURRENT_LIST_DIR}/effect/discrete_path_effect.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/discrete_path_effect.hpp
  ${CMAKE_CURRENT_LIST_DIR}/effect/gradient_shader.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/gradient_shader.hpp
  ${CMAKE_CURRENT_LIST_DIR}/effect/mask_filter.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/path_effect.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/pixmap_shader.cc
  ${CMAKE_CURRENT_LIST_DIR}/effect/pixmap_shader.hpp
  ${CMAKE_CURRENT_LIST_DIR}/effect/shader.cc
  ${CMAKE_CURRENT_LIST_DIR}/geometry/conic.cc
  ${CMAKE_CURRENT_LIST_DIR}/geometry/conic.hpp
  ${CMAKE_CURRENT_LIST_DIR}/geometry/contour_measure.cc
  ${CMAKE_CURRENT_LIST_DIR}/geometry/contour_measure.hpp
  ${CMAKE_CURRENT_LIST_DIR}/geometry/geometry.cc
  ${CMAKE_CURRENT_LIST_DIR}/geometry/geometry.hpp
  ${CMAKE_CURRENT_LIST_DIR}/geometry/math.hpp
  ${CMAKE_CURRENT_LIST_DIR}/geometry/point_priv.hpp
  ${CMAKE_CURRENT_LIST_DIR}/geometry/rect.cc
  ${CMAKE_CURRENT_LIST_DIR}/geometry/rrect.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/bitmap.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/color.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/paint.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/path.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/path_measure.cc
  ${CMAKE_CURRENT_LIST_DIR}/graphic/path_measure.hpp
  ${CMAKE_CURRENT_LIST_DIR}/graphic/path_priv.hpp
  ${CMAKE_CURRENT_LIST_DIR}/logging.cc
  ${CMAKE_CURRENT_LIST_DIR}/logging.hpp
  ${CMAKE_CURRENT_LIST_DIR}/render/canvas.cc
  ${CMAKE_CURRENT_LIST_DIR}/render/text/font_texture.cc
  ${CMAKE_CURRENT_LIST_DIR}/render/text/font_texture.hpp
  ${CMAKE_CURRENT_LIST_DIR}/render/texture_atlas.cc
  ${CMAKE_CURRENT_LIST_DIR}/render/texture_atlas.hpp
  ${CMAKE_CURRENT_LIST_DIR}/text/text_blob.cc
  ${CMAKE_CURRENT_LIST_DIR}/text/text_run.cc
  ${CMAKE_CURRENT_LIST_DIR}/text/typeface.cc
  ${CMAKE_CURRENT_LIST_DIR}/text/utf.cc
  ${CMAKE_CURRENT_LIST_DIR}/utils/lazy.hpp
)

# freetype
if(${FREETYPE_FOUND})
  target_sources(
    skity
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/text/ft_library_wrap.cc
    ${CMAKE_CURRENT_LIST_DIR}/text/ft_library_wrap.hpp
  )
endif()


# hardware backends
if(${ENABLE_HW_RENDER})

  target_sources(
    skity
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_canvas.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_canvas.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_canvas_state.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_canvas_state.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_draw.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_draw.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_font_texture.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_geometry_raster.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_geometry_raster.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_mesh.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_mesh.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_path_raster.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_path_raster.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_path_visitor.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_path_visitor.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_render_target.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_render_target.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_renderer.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/hw/hw_texture.hpp
  )

  # OpenGL backends
  if(${OPENGL_BACKEND})
    target_sources(
      skity
      PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_canvas.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_canvas.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_font_texture.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_font_texture.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_interface.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_interface.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_render_target.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_render_target.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_renderer.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_renderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_shader.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_shader.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_texture.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/gl/gl_texture.hpp
    )
  endif()

  if(${VULKAN_BACKEND})
    target_sources(
      skity
      PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/blur_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/blur_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/color_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/color_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/gradient_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/gradient_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/image_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/image_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/static_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/static_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/stencil_pipeline.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/pipelines/stencil_pipeline.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_canvas.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_canvas.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_font_texture.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_font_texture.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_framebuffer.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_framebuffer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_interface.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_interface.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_memory.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_memory.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_pipeline_wrapper.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_pipeline_wrapper.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_render_target.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_render_target.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_renderer.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_renderer.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_texture.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_texture.hpp
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_utils.cc
      ${CMAKE_CURRENT_LIST_DIR}/render/hw/vk/vk_utils.hpp
    )
  endif()

endif()

if (${CPU_BACKEND})
  target_sources(skity
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/render/sw/sw_canvas.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/sw/sw_canvas.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/sw/sw_raster.cc
    ${CMAKE_CURRENT_LIST_DIR}/render/sw/sw_raster.hpp
    ${CMAKE_CURRENT_LIST_DIR}/render/sw/sw_subpixel.hpp
  )
endif()

if(EMSCRIPTEN)
  target_sources(skity
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/render/wasm/wasm_canvas.cc
  )

  add_executable(skity_wasm ${CMAKE_CURRENT_LIST_DIR}/render/wasm/wasm_binding.cc)
  target_compile_options(skity_wasm PUBLIC "-s" "USE_FREETYPE=1")
  target_link_libraries(skity_wasm skity::skity)
  # output dir
  set_target_properties(skity_wasm PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set_target_properties(skity_wasm PROPERTIES LINK_FLAGS "-s WASM=1 -sSINGLE_FILE -sMAX_WEBGL_VERSION=2 -sFULL_ES3 -s MODULARIZE=1 -sGL_PREINITIALIZED_CONTEXT -s USE_FREETYPE=1 --bind")
endif()
