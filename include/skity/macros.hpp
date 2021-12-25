#ifndef SKITY_MACROS_HPP
#define SKITY_MACROS_HPP

#ifdef SKITY_RELEASE
#if defined(_MSC_VER)
#define SK_API __declspec(dllexport)
#else
#define SK_API __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define SK_API __declspec(dllexport)
#else
#define SK_API
#endif
#endif

#endif  // SKITY_MACROS_HPP
