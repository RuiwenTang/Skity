#include <codecvt>
#include <glm/gtc/matrix_transform.hpp>
#include <locale>
#include <skity/text/typeface.hpp>
#include <skity_config.hpp>
#include <unordered_map>
#ifdef ENABLE_TEXT_RENDER
#include "src/text/ft_library_wrap.hpp"
#endif

namespace skity {

#ifdef ENABLE_TEXT_RENDER

std::weak_ptr<FTLibrary> g_ft_library;

static std::shared_ptr<FTLibrary> GlobalFTLibrary() {
  auto library = g_ft_library.lock();
  if (library) {
    return library;
  } else {
    library = std::make_shared<FTLibrary>();
    g_ft_library = library;
    return library;
  }
}

// ---------------------------- Impl ----------------------------------
class Typeface::Impl {
 public:
  Impl() = default;
  ~Impl() = default;

  void SetFTLibrary(std::shared_ptr<FTLibrary> library) {
    ft_library_ = std::move(library);
  }

  bool LoadTypefaceFromFile(const char* path) {
    ft_typeface_ = ft_library_->LoadTypeface(path);
    return ft_typeface_ != nullptr;
  }

  bool LoadFromData(const Data* data) {
    ft_typeface_ = ft_library_->LoadTypeface(data);
    return ft_typeface_ != nullptr;
  }

  GlyphInfo GetGlyphInfo(GlyphID glyph_id, float font_size) {
    if (glyph_cache_.count(glyph_id) != 0) {
      // got catch
      return ScaleInfo(glyph_cache_[glyph_id], font_size);
    } else {
      GlyphInfo info = LoadGlyphInfo(glyph_id, font_size);
      glyph_cache_[glyph_id] = info;
      return info;
    }
  }

 private:
  GlyphInfo ScaleInfo(GlyphInfo const& base_info, float target_font_size) {
    if (target_font_size == base_info.font_size) {
      return base_info;
    }
    float scale = target_font_size / base_info.font_size;
    GlyphInfo target_info;
    target_info.id = base_info.id;
    target_info.advance_x = base_info.advance_x * scale;
    target_info.advance_y = base_info.advance_y * scale;
    target_info.ascent = base_info.ascent * scale;
    target_info.descent = base_info.descent * scale;
    target_info.width = base_info.width * scale;
    target_info.height = base_info.height * scale;
    target_info.font_size = target_font_size;


    target_info.path = base_info.path.copyWithScale(scale);
    return target_info;
  }

  GlyphInfo LoadGlyphInfo(GlyphID glyph_id, float font_size) {
    GlyphInfo glyph_info{};

    if (!ft_typeface_) {
      glyph_info.id = 0;
      return glyph_info;
    }

    auto ft_info = ft_typeface_->LoadGlyph(glyph_id, font_size);

    glyph_info.id = glyph_id;
    glyph_info.path = ft_info.path;
    glyph_info.advance_x = ft_info.advance_x;
    glyph_info.advance_y = ft_info.advance_y;
    glyph_info.ascent = ft_info.bearing_y;
    glyph_info.descent = glyph_info.height - glyph_info.ascent;

    glyph_info.font_size = font_size;

    return glyph_info;
  }

 private:
  std::shared_ptr<FTLibrary> ft_library_;
  std::unique_ptr<FTTypeFace> ft_typeface_;
  std::unordered_map<GlyphID, GlyphInfo> glyph_cache_;
};

#else
class Typeface::Impl {
 public:
 private:
};
#endif

std::unique_ptr<Typeface> Typeface::MakeDefault() {
#ifdef ENABLE_TEXT_RENDER
  std::unique_ptr<Typeface::Impl> impl{new Typeface::Impl};
  impl->SetFTLibrary(GlobalFTLibrary());

  if (!impl->LoadTypefaceFromFile(BUILD_IN_FONT_FILE)) {
    return nullptr;
  }

  std::unique_ptr<Typeface> typeface{new Typeface};
  typeface->impl_ = std::move(impl);
  return typeface;
#else
  return nullptr;
#endif
}

Typeface::~Typeface() = default;

std::unique_ptr<Typeface> Typeface::MakeFromFile(const char* path) {
#ifdef ENABLE_TEXT_RENDER
  std::unique_ptr<Typeface::Impl> impl{new Typeface::Impl};
  impl->SetFTLibrary(GlobalFTLibrary());
  if (!impl->LoadTypefaceFromFile(path)) {
    return nullptr;
  }

  std::unique_ptr<Typeface> typeface{new Typeface};
  typeface->impl_ = std::move(impl);
  return typeface;
#else
  return nullptr;
#endif
}

std::unique_ptr<Typeface> Typeface::MakeFromData(
    const std::shared_ptr<Data>& data) {
#ifdef ENABLE_TEXT_RENDER
  std::unique_ptr<Typeface::Impl> impl{new Typeface::Impl};
  impl->SetFTLibrary(GlobalFTLibrary());
  if (!impl->LoadFromData(data.get())) {
    return nullptr;
  }

  std::unique_ptr<Typeface> typeface{new Typeface};
  typeface->impl_ = std::move(impl);
  return typeface;
#else
  return nullptr;
#endif
}

void Typeface::textToGlyphId(const char* text, std::vector<GlyphID>& glyphs) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32_conv;

  auto utf32 = utf32_conv.from_bytes(text);
  for (auto c : utf32) {
    glyphs.emplace_back(c);
  }
}

void Typeface::textToGlyphInfo(const char* text, float font_size,
                               std::vector<GlyphInfo>& info) {
  std::vector<GlyphID> glyph_id;
  textToGlyphId(text, glyph_id);

  for (auto id : glyph_id) {
    info.emplace_back(getGlyphInfo(id, font_size));
  }
}

void Typeface::getGlyphInfo(const std::vector<GlyphID>& glyph_id,
                            float font_size, std::vector<GlyphInfo>& info) {
  for (auto id : glyph_id) {
    info.emplace_back(getGlyphInfo(id, font_size));
  }
}

GlyphInfo Typeface::getGlyphInfo(GlyphID glyph_id, float font_size) {
  return impl_->GetGlyphInfo(glyph_id, font_size);
}

}  // namespace skity
