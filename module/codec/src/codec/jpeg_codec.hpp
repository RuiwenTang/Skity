#ifndef SKITY_SRC_CODEC_JPEG_CODEC_HPP
#define SKITY_SRC_CODEC_JPEG_CODEC_HPP

#ifdef SKITY_HAS_JPEG

#include <skity/codec/codec.hpp>

namespace skity {

class JPEGCodec : public Codec {
 public:
  JPEGCodec() = default;
  ~JPEGCodec() override = default;

  std::shared_ptr<Pixmap> Decode() override;

  std::shared_ptr<Data> Encode(const Pixmap* pixmap) override;

  bool RecognizeFileType(const char* header, size_t size) override;
};

}  // namespace skity

#endif  // SKITY_HAS_JPEG

#endif  // SKITY_SRC_CODEC_JPEG_CODEC_HPP
