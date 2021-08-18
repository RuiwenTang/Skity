#include <cassert>
#include <cstring>
#include <functional>
#include <mutex>
#include <skity/codec/data.hpp>

namespace skity {

static void skity_free_releaseproc(const void* ptr, void*) {
  std::free((void*)ptr);
}

Data::Data(const void* ptr, size_t size, ReleaseProc proc, void* context)
    : ptr_(ptr), size_(size), proc_(proc), context_(context) {}

Data::~Data() {
  if (proc_) {
    proc_(ptr_, context_);
  }
}

std::shared_ptr<Data> Data::PrivateNewWithCopy(const void* srcOrNull,
                                               size_t length) {
  if (0 == length) {
    return Data::MakeEmpty();
  }

  if (srcOrNull == nullptr) {
    return Data::MakeEmpty();
  }

  void* data = std::malloc(length);
  std::memcpy(data, srcOrNull, length);

  return Data::MakeFromMalloc(data, length);
}

std::shared_ptr<Data> Data::MakeEmpty() {
  static std::shared_ptr<Data> empty;
  std::once_flag flag;
  std::call_once(flag,
                 [] { empty.reset(new Data(nullptr, 0, nullptr, nullptr)); });
  return empty;
}

std::shared_ptr<Data> Data::MakeWithCopy(const void* data, size_t length) {
  assert(data);
  return PrivateNewWithCopy(data, length);
}

std::shared_ptr<Data> Data::MakeWithCString(const char* cStr) {
  size_t size;
  if (nullptr == cStr) {
    cStr = "";
    size = 1;
  } else {
    size = std::strlen(cStr) + 1;
  }

  return MakeWithCopy(cStr, size);
}

std::shared_ptr<Data> Data::MakeWithProc(const void* ptr, size_t length,
                                         ReleaseProc proc, void* ctx) {
  return std::shared_ptr<Data>(new Data(ptr, length, proc, ctx));
}

std::shared_ptr<Data> Data::MakeFromMalloc(const void* data, size_t length) {
  return std::shared_ptr<Data>(
      new Data(data, length, skity_free_releaseproc, nullptr));
}

}  // namespace skity