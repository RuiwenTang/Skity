#include "src/render/gl/gl_glyph_raster_cache.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "src/render/gl/gl_fill.hpp"
#include "src/render/gl/gl_stroke.hpp"
#include "src/render/gl/gl_stroke_aa.hpp"

namespace skity {

void GLGlyphRasterCache::MergeGlyphCache(GLVertex *gl_vertex,
                                         const std::vector<GlyphInfo> &glyphs,
                                         const Typeface *typeface,
                                         const Paint &paint, bool fill) {
  float advance_x = 0.f;
  float target_font_size = paint.getTextSize();
  for (const auto &glyph : glyphs) {
    GlyphCacheItem *cache_item = FindRasterCache(glyph.id, typeface, fill);
    if (cache_item == nullptr) {
      cache_item =
          GenerateRasterCache(glyph, typeface, paint, target_font_size, fill);
    }
    float scale = target_font_size / cache_item->font_size;

    MergeVertex(
        gl_vertex,
        fill ? cache_item->fill_cache.get() : cache_item->stroke_cache.get(),
        advance_x, scale);
    advance_x += cache_item->advance_x * scale;
  }
}

GlyphCacheItem *GLGlyphRasterCache::GenerateRasterCache(
    const GlyphInfo &info, const Typeface *typeface, Paint const &paint,
    float target_font_size, bool fill) {
  float scale = target_font_size / info.font_size;

  Path temp = info.path.copyWithScale(scale);

  GlyphCacheItem *item = FindRasterCache(info.id, typeface, !fill);

  if (!item) {
    // first meet this glyph id with this typeface
    item = InsertNewCacheItem(info.id, typeface);
    item->typeface = typeface;
    item->font_size = target_font_size;
    item->advance_x = info.advance_x;
  }

  if (fill) {
    item->fill_cache = RasterGlyphPath(temp, paint, fill, paint.isAntiAlias());
  } else {
    item->stroke_cache =
        RasterGlyphPath(temp, paint, fill, paint.isAntiAlias());
  }

  return item;
}

void GLGlyphRasterCache::MergeVertex(GLVertex *target, GLVertex *src,
                                     float advance_x, float scale) {
  // Matrix matrix = glm::identity<Matrix>();

  // if (scale != 1.f) {
  //   matrix = glm::scale(matrix, Vec3{scale, scale, 1.f});
  // }

  // matrix = glm::translate(glm::identity<Matrix>(), Vec3{advance_x, 0.f, 0.f})
  // *
  //          matrix;

  // target->Append(src, matrix);
  target->Append(src, scale, advance_x, 0.f);
}

std::unique_ptr<GLVertex> GLGlyphRasterCache::RasterGlyphPath(
    Path const &path, Paint const &paint, bool fill, bool aa) {
  std::unique_ptr<GLVertex> vertex = std::make_unique<GLVertex>();

  if (fill) {
    GLFill gl_fill;
    gl_fill.fillPath(path, paint, vertex.get());
    if (aa) {
      GLStrokeAA gl_stroke_aa(1.f);
      gl_stroke_aa.StrokePathAA(path, vertex.get());
    }
  } else {
    GLStroke gl_stroke(paint);
    gl_stroke.strokePath(path, vertex.get());
  }

  return vertex;
}

GlyphCacheItem *GLGlyphRasterCache::FindRasterCache(GlyphID glyph_id,
                                                    const Typeface *typeface,
                                                    bool fill) {
  if (caches_.count(glyph_id) == 0) {
    return nullptr;
  }

  const auto &cache_list = caches_[glyph_id];
  for (auto const &item : cache_list) {
    if (item.typeface == typeface) {
      if ((fill && item.fill_cache) || (!fill && item.stroke_cache)) {
        return const_cast<GlyphCacheItem *>(std::addressof(item));
      } else {
        return nullptr;
      }
    }
  }

  return nullptr;
}

GlyphCacheItem *GLGlyphRasterCache::InsertNewCacheItem(
    GlyphID glyph_id, const Typeface *typeface) {
  if (caches_.count(glyph_id) == 0) {
    caches_[glyph_id] = std::vector<GlyphCacheItem>();
  }

  auto &item_list = caches_[glyph_id];
  item_list.emplace_back(GlyphCacheItem{typeface});

  return std::addressof(item_list.back());
}

}  // namespace skity
