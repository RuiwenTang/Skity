#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <vector>

#ifdef SKITY_HAS_PNG
#include "src/codec/png_codec.hpp"
#endif

namespace skity {

static std::vector<std::shared_ptr<Codec>> codec_list = {};

void Codec::SetupCodecs() {
  codec_list.clear();

#ifdef SKITY_HAS_PNG
  codec_list.emplace_back(std::make_shared<PNGCodec>());
#endif
}

std::shared_ptr<Codec> Codec::MakeFromData(const std::shared_ptr<Data>& data) {
  if (codec_list.empty()) {
    SetupCodecs();
  }

  if (!data || data->Size() <= 20) {
    return nullptr;
  }

  const char* header = reinterpret_cast<const char*>(data->RawData());

  for (auto const& codec : codec_list) {
    if (codec->RecognizeFileType(header, 20)) {
      return codec;
    }
  }

  return nullptr;
}

}  // namespace skity