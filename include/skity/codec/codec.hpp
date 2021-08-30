#ifndef SKITY_CODEC_CODEC_HPP
#define SKITY_CODEC_CODEC_HPP

#include <memory>
#include <skity/macros.hpp>
#include <utility>

namespace skity {

class Data;
class Pixmap;

/**
 * Codec interface
 */
class SK_API Codec {
 public:
  Codec() = default;
  virtual ~Codec() = default;

  virtual std::shared_ptr<Pixmap> Decode() = 0;

  virtual std::shared_ptr<Data> Encode() = 0;

  virtual bool RecognizeFileType(const char* header, size_t size) = 0;

  void SetData(std::shared_ptr<Data> data) { data_ = std::move(data); }

  static std::shared_ptr<Codec> MakeFromData(std::shared_ptr<Data> const& data);

 protected:
  std::shared_ptr<Data> data_;

 private:
  static void SetupCodecs();
};

}  // namespace skity

#endif  // SKITY_CODEC_CODEC_HPP
