#include <skity/codec/codec.hpp>
#include <skity/skity.hpp>

int main(int argc, const char** argv) {
  auto codec = skity::Codec::MakeJPEGCodec();
  
  size_t len = 400 * 400 * 4;
  auto data = skity::Data::MakeFromMalloc(std::malloc(len), len);

  uint32_t* ptr = (uint32_t*)data->RawData();
  for (size_t i = 0; i < 400; i++) {
    for (size_t j = 0; j < 400; j++) {
      ptr[i * 400 + j] = skity::ColorSetARGB(255, 255, 0, 0);
    }
  }

  auto pixmap = std::make_shared<skity::Pixmap>(data, 400 * 4, 400, 400);

  auto encoded_data = codec->Encode(pixmap.get());
  
  auto ret = encoded_data->WriteToFile("jpeg_encode_test.jpg");

  return 0;
}