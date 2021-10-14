#ifndef SRC_RENDER_GL_GL_GLYPH_RASTER_CACHE_HPP
#define SRC_RENDER_GL_GL_GLYPH_RASTER_CACHE_HPP

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/text/typeface.hpp>
#include <unordered_map>
#include <vector>

#include "src/render/gl/gl_vertex.hpp"

namespace skity {

struct GlyphCacheItem {
  const Typeface* typeface;
  float font_size;
  float advance_x;
  std::unique_ptr<GLVertex> fill_cache;
  std::unique_ptr<GLVertex> stroke_cache;
};

class GLGlyphRasterCache final {
 public:
  GLGlyphRasterCache() = default;
  ~GLGlyphRasterCache() = default;

  void MergeGlyphCache(GLVertex* gl_vertex,
                       std::vector<GlyphInfo> const& glyphs,
                       const Typeface* typeface, Paint const& paint, bool fill);

 private:
  void MergeVertex(GLVertex* target, GLVertex* src, float advance_x,
                   float scale);
  std::unique_ptr<GLVertex> RasterGlyphPath(Path const& path,
                                            Paint const& paint, bool fill,
                                            bool aa);

  GlyphCacheItem* GenerateRasterCache(GlyphInfo const& info,
                                      const Typeface* typeface,
                                      Paint const& paint,
                                      float target_font_size, bool fill);

  GlyphCacheItem* FindRasterCache(GlyphID glyph_id, const Typeface* typeface,
                                  bool fill);

  GlyphCacheItem* InsertNewCacheItem(GlyphID glyph_id,
                                     const Typeface* typeface);

 private:
  std::unordered_map<GlyphID, std::vector<GlyphCacheItem>> caches_;
};

}  // namespace skity

#endif  // SRC_RENDER_GL_GL_GLYPH_RASTER_CACHE_HPP
