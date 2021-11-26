#ifndef SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP
#define SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP

#include <GL/glcorearb.h>

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
  PFNGLBINDSAMPLERPROC fBindSampler = nullptr;
  PFNGLBINDTEXTUREPROC fBindTexture = nullptr;
  PFNGLBINDVERTEXARRAYPROC fBindVertexArray = nullptr;
  PFNGLBLENDCOLORPROC fBlendColor = nullptr;
  PFNGLBLENDEQUATIONPROC fBlendEquation = nullptr;
  PFNGLBLENDFUNCPROC fBlend = nullptr;
  PFNGLBUFFERDATAPROC fBufferData = nullptr;
  PFNGLBUFFERSUBDATAPROC fBufferSubData = nullptr;
  PFNGLCHECKFRAMEBUFFERSTATUSPROC fCheckFramebufferStatus = nullptr;
  PFNGLCLEARPROC fClear = nullptr;
  PFNGLCLEARCOLORPROC fClearColor = nullptr;
  PFNGLCLEARSTENCILPROC fClearStencil = nullptr;
  PFNGLCOLORMASKPROC fColorMask = nullptr;
  PFNGLCOMPILESHADERPROC fCompileShader = nullptr;
  PFNGLCREATEPROGRAMPROC fCreateProgram = nullptr;
  PFNGLCREATESHADERPROC fCreateShader = nullptr;
  PFNGLCULLFACEPROC fCullFace = nullptr;
  PFNGLDELETEBUFFERSPROC fDeleteBuffers = nullptr;
  PFNGLDELETEFRAMEBUFFERSPROC fDeleteFramebuffers = nullptr;
  PFNGLDELETERENDERBUFFERSPROC fDeleteRenderbuffers = nullptr;
  PFNGLDELETESHADERPROC fDeleteShader = nullptr;
  PFNGLDELETEPROGRAMPROC fDeleteProgram = nullptr;
  PFNGLDELETETEXTURESPROC fDeleteTextures = nullptr;
  PFNGLDELETEVERTEXARRAYSPROC fDeleteVertexArrays = nullptr;
  PFNGLDEPTHMASKPROC fDepthMask = nullptr;
  PFNGLDISABLEPROC fDisable = nullptr;
  PFNGLENABLEVERTEXATTRIBARRAYPROC fEnableVertexAttribArray = nullptr;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC fDisableVertexAttribArray = nullptr;
  PFNGLVERTEXATTRIBPOINTERPROC fVertexAttribPointer = nullptr;
  PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC fDisableVertexArrayAttrib = nullptr;
  PFNGLDRAWARRAYSPROC fDrawArrays = nullptr;
  PFNGLDRAWARRAYSINDIRECTPROC fDrawArraysIndirect = nullptr;
  PFNGLDRAWARRAYSINSTANCEDPROC fDrawArraysInstanced = nullptr;
  PFNGLDRAWBUFFERPROC fDrawBuffer = nullptr;
  PFNGLDRAWBUFFERSPROC fDrawBuffers = nullptr;
  PFNGLDRAWELEMENTSPROC fDrawElements = nullptr;
  PFNGLENABLEPROC fEnable = nullptr;
  PFNGLFLUSHPROC fFlush = nullptr;
  PFNGLGENBUFFERSPROC fGenBuffers = nullptr;
  PFNGLGENTEXTURESPROC fGenTextures = nullptr;
  PFNGLGENVERTEXARRAYSPROC fGenVertexArrays = nullptr;
  PFNGLGETERRORPROC fGetError = nullptr;
  PFNGLGETSHADERINFOLOGPROC fGetShaderInfoLog = nullptr;
  PFNGLGETPROGRAMINFOLOGPROC fGetProgramInfoLog = nullptr;
  PFNGLGETSHADERIVPROC fGetShaderiv = nullptr;
  PFNGLGETPROGRAMIVPROC fGetProgramiv = nullptr;
  PFNGLGETUNIFORMLOCATIONPROC fGetUniformLocation = nullptr;
  PFNGLLINKPROGRAMPROC fLinkProgram = nullptr;
  PFNGLSHADERSOURCEPROC fShaderSource = nullptr;
  PFNGLSTENCILFUNCPROC fStencilFunc = nullptr;
  PFNGLSTENCILMASKPROC fStencilMask = nullptr;
  PFNGLSTENCILOPPROC fStencilOp = nullptr;
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
  PFNGLUNIFORM4IPROC fUniform4i = nullptr;
  PFNGLUNIFORM4FVPROC fUniform4fv = nullptr;
  PFNGLUNIFORMMATRIX4FVPROC fUniformMatrix4fv = nullptr;
  PFNGLUSEPROGRAMPROC fUseProgram = nullptr;
  PFNGLTEXPARAMETERIPROC fTexParameteri = nullptr;
  PFNGLTEXIMAGE2DPROC fTexImage2D = nullptr;
  PFNGLTEXSUBIMAGE2DPROC fTexSubImage2D = nullptr;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_GL_GL_INTERFACE_HPP
