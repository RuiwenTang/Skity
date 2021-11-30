#ifndef SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP
#define SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <skity/text/typeface.hpp>

#include "src/render/texture_atlas.hpp"

namespace skity {

class FontTexture : public TextureAtlas {
  enum {
    DEFAULT_SIZE = 512,
  };

  struct GlyphKey {
    GlyphID id;
    float font_size;

    GlyphKey() = default;
    ~GlyphKey() = default;

    bool operator==(GlyphKey const& other) const;
  };

  struct GlyphKeyCompare {
    bool operator()(GlyphKey const& lhs, GlyphKey const& rhs) const {
      if (lhs.id < rhs.id) {
        return true;
      }

      if (lhs.id == rhs.id) {
        return lhs.font_size < rhs.font_size;
      }

      return false;
    }
  };

 public:
  FontTexture(Typeface* typeface);
  ~FontTexture() override = default;

  glm::ivec4 GetGlyphRegion(GlyphID glyph_id, float font_size);

 private:
  glm::ivec4 GenerateGlyphRegion(GlyphKey const& key);

 private:
  Typeface* typeface_;
  std::map<GlyphKey, glm::ivec4, GlyphKeyCompare> glyph_regions_ = {};
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_TEXT_FONT_TEXTURE_HPP