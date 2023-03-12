#ifndef SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP
#define SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP

#if defined(__ANDROID__) || defined(ANDROID)
#define SKITY_USE_GLES3 1
#ifdef SKITY_USE_GLES3
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#define PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC PFNGLDISABLEVERTEXATTRIBARRAYPROC
#else
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define PFNGLGENVERTEXARRAYSPROC PFNGLGENVERTEXARRAYSOESPROC
#define PFNGLDELETEVERTEXARRAYSPROC PFNGLDELETEVERTEXARRAYSOESPROC
#define PFNGLBINDVERTEXARRAYPROC PFNGLBINDVERTEXARRAYOESPROC
#define PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC PFNGLDISABLEVERTEXATTRIBARRAYPROC
#define PFNGLDRAWARRAYSINSTANCEDPROC  PFNGLDRAWARRAYSINSTANCEDEXTPROC
#define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
#define GL_DEPTH24_STENCIL8 GL_DEPTH24_STENCIL8_OES
#define GL_R8 GL_R8_EXT
#define GL_RED GL_RED_EXT
#define GL_RGBA8 GL_RGBA8_OES
#endif
#else
#include <GL/glcorearb.h>
#endif


namespace skity {

#define GL_CALL(name, ...) GLInterface::GlobalInterface()->f##name(__VA_ARGS__)

struct GLInterface {
  typedef void (*GLFuncPtr)();
  typedef GLFuncPtr (*GLGetProc)(const char* name);

  static GLInterface* GlobalInterface();
  static void InitGlobalInterface(void* proc_loader);

  PFNGLACTIVETEXTUREPROC fActiveTexture = nullptr;
  PFNGLATTACHSHADERPROC fAttachShader = nullptr;
  PFNGLBINDATTRIBLOCATIONPROC fBindAttribLocation = nullptr;
  PFNGLBINDBUFFERPROC fBindBuffer = nullptr;
  PFNGLBINDFRAMEBUFFERPROC fBindFramebuffer = nullptr;
  PFNGLBINDRENDERBUFFERPROC fBindRenderbuffer = nullptr;
#if !defined(__ANDROID__) && !defined(ANDROID)
  PFNGLBINDSAMPLERPROC fBindSampler = nullptr;
#endif
  PFNGLBINDTEXTUREPROC fBindTexture = nullptr;
  PFNGLBINDVERTEXARRAYPROC fBindVertexArray = nullptr;
  PFNGLBLENDCOLORPROC fBlendColor = nullptr;
  PFNGLBLENDEQUATIONPROC fBlendEquation = nullptr;
  PFNGLBLENDFUNCPROC fBlend = nullptr;
  PFNGLBUFFERDATAPROC fBufferData = nullptr;
  PFNGLBUFFERSUBDATAPROC fBufferSubData = nullptr;
  PFNGLCHECKFRAMEBUFFERSTATUSPROC fCheckFramebufferStatus = nullptr;
  PFNGLCLEARCOLORPROC fClearColor = nullptr;
  PFNGLCLEARPROC fClear = nullptr;
  PFNGLCLEARSTENCILPROC fClearStencil = nullptr;
  PFNGLCOLORMASKPROC fColorMask = nullptr;
  PFNGLCOMPILESHADERPROC fCompileShader = nullptr;
  PFNGLGENFRAMEBUFFERSPROC fGenFramebuffers = nullptr;
  PFNGLCREATEPROGRAMPROC fCreateProgram = nullptr;
  PFNGLCREATESHADERPROC fCreateShader = nullptr;
  PFNGLCULLFACEPROC fCullFace = nullptr;
  PFNGLDELETEBUFFERSPROC fDeleteBuffers = nullptr;
  PFNGLDELETEFRAMEBUFFERSPROC fDeleteFramebuffers = nullptr;
  PFNGLDELETEPROGRAMPROC fDeleteProgram = nullptr;
  PFNGLDELETERENDERBUFFERSPROC fDeleteRenderbuffers = nullptr;
  PFNGLDELETESHADERPROC fDeleteShader = nullptr;
  PFNGLDELETETEXTURESPROC fDeleteTextures = nullptr;
  PFNGLDELETEVERTEXARRAYSPROC fDeleteVertexArrays = nullptr;
  PFNGLDEPTHMASKPROC fDepthMask = nullptr;
  PFNGLDISABLEPROC fDisable = nullptr;
  PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC fDisableVertexArrayAttrib = nullptr;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC fDisableVertexAttribArray = nullptr;

#if !defined(__ANDROID__) && !defined(ANDROID)
  PFNGLDRAWARRAYSINDIRECTPROC fDrawArraysIndirect = nullptr;
#endif
  PFNGLDRAWARRAYSPROC fDrawArrays = nullptr;
#if !defined(__ANDROID__) && !defined(ANDROID)
  PFNGLDRAWARRAYSINSTANCEDPROC fDrawArraysInstanced = nullptr;
  PFNGLDRAWBUFFERPROC fDrawBuffer = nullptr;
  PFNGLDRAWBUFFERSPROC fDrawBuffers = nullptr;
#endif
  PFNGLDRAWELEMENTSPROC fDrawElements = nullptr;
  PFNGLENABLEPROC fEnable = nullptr;
  PFNGLENABLEVERTEXATTRIBARRAYPROC fEnableVertexAttribArray = nullptr;
  PFNGLFLUSHPROC fFlush = nullptr;
  PFNGLFRAMEBUFFERTEXTURE2DPROC fFramebufferTexture2D = nullptr;
  PFNGLGENBUFFERSPROC fGenBuffers = nullptr;
  PFNGLGENTEXTURESPROC fGenTextures = nullptr;
  PFNGLGENVERTEXARRAYSPROC fGenVertexArrays = nullptr;
  PFNGLGETERRORPROC fGetError = nullptr;
  PFNGLGETPROGRAMINFOLOGPROC fGetProgramInfoLog = nullptr;
  PFNGLGETPROGRAMIVPROC fGetProgramiv = nullptr;
  PFNGLGETSHADERINFOLOGPROC fGetShaderInfoLog = nullptr;
  PFNGLGETSHADERIVPROC fGetShaderiv = nullptr;
  PFNGLGETUNIFORMLOCATIONPROC fGetUniformLocation = nullptr;
  PFNGLLINKPROGRAMPROC fLinkProgram = nullptr;
  PFNGLPIXELSTOREIPROC fPixelStorei = nullptr;
  PFNGLSHADERSOURCEPROC fShaderSource = nullptr;
  PFNGLSTENCILFUNCPROC fStencilFunc = nullptr;
  PFNGLSTENCILMASKPROC fStencilMask = nullptr;
  PFNGLSTENCILOPPROC fStencilOp = nullptr;
  PFNGLTEXIMAGE2DPROC fTexImage2D = nullptr;
#if !defined(__ANDROID__) && !defined(ANDROID)
  PFNGLTEXIMAGE2DMULTISAMPLEPROC fTexImage2DMultisample = nullptr;
  PFNGLBLITFRAMEBUFFERPROC fBlitFramebuffer = nullptr;
#else
#ifdef SKITY_USE_GLES3
        PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC fRenderbufferStorageMultisample = nullptr;
        PFNGLBLITFRAMEBUFFERPROC fBlitFramebuffer = nullptr;
#endif
#endif
  PFNGLTEXPARAMETERIPROC fTexParameteri = nullptr;
  PFNGLTEXSUBIMAGE2DPROC fTexSubImage2D = nullptr;
  PFNGLUNIFORM1FPROC fUniform1f = nullptr;
  PFNGLUNIFORM1FVPROC fUniform1fv = nullptr;
  PFNGLUNIFORM1IPROC fUniform1i = nullptr;
  PFNGLUNIFORM1IVPROC fUniform1iv = nullptr;
  PFNGLUNIFORM2FPROC fUniform2f = nullptr;
  PFNGLUNIFORM2FVPROC fUniform2fv = nullptr;
  PFNGLUNIFORM2IPROC fUniform2i = nullptr;
  PFNGLUNIFORM2IVPROC fUniform2iv = nullptr;
  PFNGLUNIFORM3FPROC fUniform3f = nullptr;
  PFNGLUNIFORM3FVPROC fUniform3fv = nullptr;
  PFNGLUNIFORM4FPROC fUniform4f = nullptr;
  PFNGLUNIFORM4FVPROC fUniform4fv = nullptr;
  PFNGLUNIFORM4IPROC fUniform4i = nullptr;
  PFNGLUNIFORMMATRIX4FVPROC fUniformMatrix4fv = nullptr;
  PFNGLUSEPROGRAMPROC fUseProgram = nullptr;
  PFNGLVERTEXATTRIBPOINTERPROC fVertexAttribPointer = nullptr;
#if !defined(__ANDROID__) && !defined(ANDROID)
  PFNGLCLEARBUFFERFVPROC fClearBufferfv = nullptr;
  PFNGLCLEARBUFFERFIPROC fClearBufferfi = nullptr;
#else

#endif
  PFNGLVIEWPORTPROC fViewport = nullptr;
  PFNGLGETINTEGERVPROC fGetIntegerv = nullptr;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP
