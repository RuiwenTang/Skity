#ifndef SKITY_SRC_CODEC_PNG_CODEC_HPP
#define SKITY_SRC_CODEC_PNG_CODEC_HPP

#include <skity/codec/codec.hpp>

#ifdef SKITY_HAS_PNG
#include <png.h>

namespace skity {

class PNGCodec : public Codec {
 public:
  PNGCodec();
  ~PNGCodec() override;
  std::shared_ptr<Pixmap> Decode() override;
  std::shared_ptr<Data> Encode() override;
  bool RecognizeFileType(const char *header, size_t size) override;

 private:
  png_image image_ = {};
  std::shared_ptr<Pixmap> pixmap_ = {};
};

}  // namespace skity

#endif  // SKITY_HAS_PNG

#endif  // SKITY_SRC_CODEC_PNG_CODEC_HPP
