#include <fstream>
#include <iostream>
#include <skity/codec/codec.hpp>
#include <skity/codec/data.hpp>
#include <skity/codec/pixmap.hpp>
#include <vector>

// ./png_decode_test path_to_png_file
int main(int argc, const char** argv) {
  if (argc < 2) {
    std::cerr << "not provide png file path" << std::endl;
    return -1;
  }
  const char* file_path = argv[1];

  std::cout << "png file_path = " << file_path << std::endl;

  std::ifstream in_stream{file_path, std::ios::in | std::ios::binary};
  std::vector<uint8_t> raw_file_data(
      (std::istreambuf_iterator<char>(in_stream)),
      std::istreambuf_iterator<char>());

  auto skity_data =
      skity::Data::MakeWithCopy(raw_file_data.data(), raw_file_data.size());

  auto codec = skity::Codec::MakeFromData(skity_data);

  if (!codec) {
    std::cerr << "can not recognize png file codec" << std::endl;
  } else {
    std::cout << "png codec create success" << std::endl;
  }

  codec->SetData(skity_data);
  auto pixmap = codec->Decode();

  if (!pixmap) {
    std::cerr << "png decode failed " << std::endl;
  } else {
    std::cout << "png decode success" << std::endl;

    std::cout << "png image size = {" << pixmap->Width() << " , " << pixmap->Height() << "}" << std::endl;
  }

  return 0;
}