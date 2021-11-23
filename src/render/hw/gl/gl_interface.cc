#include "src/render/hw/gl/gl_interface.hpp"

namespace skity {

GLInterface* g_interface = nullptr;

#define GET_PROC(F) \
  g_interface->f##F = (decltype(g_interface->f##F))loader("gl" #F)

void GLInterface::InitGlobalInterface(void* proc_loader) {
  g_interface = new GLInterface;

  GLGetProc loader = (GLGetProc)proc_loader;

  GET_PROC(ActiveTexture);
  GET_PROC(AttachShader);
  GET_PROC(BindAttribLocation);
  GET_PROC(BindBuffer);
  GET_PROC(BindFramebuffer);
  GET_PROC(BindRenderbuffer);
  GET_PROC(BindSampler);
  GET_PROC(BindTexture);
  GET_PROC(BindVertexArray);
  GET_PROC(BlendColor);
  GET_PROC(BlendEquation);
  GET_PROC(Blend);
  GET_PROC(BufferData);
  GET_PROC(BufferSubData);
  GET_PROC(CheckFramebufferStatus);
  GET_PROC(Clear);
  GET_PROC(ClearColor);
  GET_PROC(ClearStencil);
  GET_PROC(ColorMask);
  GET_PROC(CompileShader);
  GET_PROC(CreateProgram);
  GET_PROC(CreateShader);
  GET_PROC(CullFace);
  GET_PROC(DeleteBuffers);
  GET_PROC(DeleteFramebuffers);
  GET_PROC(DeleteRenderbuffers);
  GET_PROC(DeleteShader);
  GET_PROC(DeleteProgram);
  GET_PROC(DeleteTextures);
  GET_PROC(DeleteVertexArrays);
  GET_PROC(DepthMask);
  GET_PROC(Disable);
  GET_PROC(EnableVertexAttribArray);
  GET_PROC(DisableVertexAttribArray);
  GET_PROC(VertexAttribPointer);
  GET_PROC(DisableVertexArrayAttrib);
  GET_PROC(DrawArrays);
  GET_PROC(DrawArraysIndirect);
  GET_PROC(DrawArraysInstanced);
  GET_PROC(DrawBuffer);
  GET_PROC(DrawBuffers);
  GET_PROC(DrawElements);
  GET_PROC(Enable);
  GET_PROC(Flush);
  GET_PROC(GenBuffers);
  GET_PROC(GenTextures);
  GET_PROC(GenVertexArrays);
  GET_PROC(GetError);
  GET_PROC(GetShaderInfoLog);
  GET_PROC(GetProgramInfoLog);
  GET_PROC(GetShaderiv);
  GET_PROC(GetProgramiv);
  GET_PROC(GetUniformLocation);
  GET_PROC(LinkProgram);
  GET_PROC(ShaderSource);
  GET_PROC(StencilFunc);
  GET_PROC(StencilMask);
  GET_PROC(StencilOp);
  GET_PROC(Uniform1f);
  GET_PROC(Uniform1fv);
  GET_PROC(Uniform1i);
  GET_PROC(Uniform1iv);
  GET_PROC(Uniform2i);
  GET_PROC(Uniform2f);
  GET_PROC(Uniform2fv);
  GET_PROC(Uniform3f);
  GET_PROC(Uniform3fv);
  GET_PROC(Uniform4f);
  GET_PROC(Uniform4i);
  GET_PROC(Uniform4fv);
  GET_PROC(UniformMatrix4fv);
  GET_PROC(UseProgram);
  GET_PROC(TexParameteri);
  GET_PROC(TexImage2D);
}

GLInterface* GLInterface::GlobalInterface() { return g_interface; }

}  // namespace skity
