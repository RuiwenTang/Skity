#ifndef SKITY_GPU_GPU_CONTEXT_HPP
#define SKITY_GPU_GPU_CONTEXT_HPP

#include <skity/config.hpp>

namespace skity {

enum class GPUBackendType {
  kNone,
  kOpenGL,
  kVulkan,
  kWebGL2,
  kMetal,
};

/**
 * @struct GPUContext
 *
 * Hold GPU information for internal create render backend
 *
 */
struct GPUContext {
  /**
   * Indicate the GPUContext type, default is **kNone**
   * use **kOpenGL** if need to create OpenGL backend renderer
   *
   */
  GPUBackendType type = GPUBackendType::kNone;

  /**
   * Function pointer which is pointer to a **GLProcLoader**
   * since **Skity** not link OpenGL library at compile time,
   * this is used to load all needed OpenGL function in runtime.
   *
   */
  void* proc_loader = nullptr;

  GPUContext(GPUBackendType type, void* proc_loader)
      : type(type), proc_loader(proc_loader) {}
};

}  // namespace skity

#endif  // SKITY_GPU_GPU_CONTEXT_HPP
