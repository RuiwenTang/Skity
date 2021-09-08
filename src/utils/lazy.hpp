#ifndef SKITY_UTILS_LAZY_HPP
#define SKITY_UTILS_LAZY_HPP

#include <new>
#include <type_traits>
#include <utility>

namespace skity {

/**
 * Helper class copied from SkTLazy.h from skia
 */
template <typename T>
class Lazy {
 public:
  Lazy() = default;
  explicit Lazy(const T* src) : ptr_(src ? new (&storage) T(*src) : nullptr) {}
  Lazy(const Lazy& that)
      : ptr_(that.ptr_ ? new (&storage) T(*that.ptr_) : nullptr) {}
  Lazy(Lazy&& that) noexcept
      : ptr_(that.ptr_ ? new (&storage) T(std::move(*that.ptr_)) : nullptr) {}
  ~Lazy() { this->Reset(); }

  Lazy& operator=(const Lazy& that) {
    if (that.IsValid()) {
      this->Set(*that);
    } else {
      this->Reset();
    }
    return *this;
  }

  Lazy& operator=(Lazy&& that) {
    if (that.IsValid()) {
      this->Set(std::move(*that));
    } else {
      this->Reset();
    }
    return *this;
  }

  void Reset() {
    if (this->IsValid()) {
      ptr_->~T();
      ptr_ = nullptr;
    }
  }

  bool IsValid() const { return ptr_ != nullptr; }

  T* Set(const T& src) {
    if (this->IsValid()) {
      *ptr_ = src;
    } else {
      ptr_ = new (&storage) T(src);
    }
    return ptr_;
  }

  template <typename... Args>
  T* Init(Args&&... args) {
    this->Reset();
    ptr_ = new (&storage) T(std::forward<Args>(args)...);
    return ptr_;
  }

  T* Set(T&& src) {
    if (this->IsValid()) {
      *ptr_ = std::move(src);
    } else {
      ptr_ = new (&storage) T(std::move(src));
    }
    return ptr_;
  }

  T* get() const { return ptr_; }
  T* operator->() const { return this->get(); }
  T& operator*() const { return *this->get(); }
  T* GetMaybeNull() const { return ptr_; }

 private:
  alignas(T) char storage[sizeof(T)] = {};
  T* ptr_ = nullptr;
};

}  // namespace skity

#endif  // SKITY_UTILS_LAZY_HPP
