
#define PIPELINE_MODE_STENCIL 0
#define PIPELINE_MODE_UNIFORM_COLOR 1
#define PIPELINE_MODE_IMAGE_TEXTURE 2
#define PIPELINE_MODE_LINEAR_GRADIENT 3
#define PIPELINE_MODE_RADIAL_GRADIENT 4

#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_CIRCLE 2
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4
#define VERTEX_TYPE_TEXT 5

// global pipeline info
layout(push_constant) uniform constants {
  mat4 mvp;
  int premulAlpha;
}
GlobalInfo;

layout(set = 0, binding = 0) uniform _Transform { mat4 uMatrix; }
TransformData;

// set 1 is fragment common
layout(set = 1, binding = 0) uniform _GlobalAlpha {
  // [global_alpha, stroke_width, TBD, TBD]
  vec4 info;
}
AlphaStroke;

// vertex input
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec3 vPosInfo;

layout(location = 0) out vec4 outColor;

float StrokeWidth() { return AlphaStroke.info.g; }

void calculate_discard() {
  int vertex_type = int(vPosInfo.x);

  if (vertex_type == VERTEX_TYPE_CIRCLE) {
    float r = length(vPos - vPosInfo.yz);
    if (r > AlphaStroke.info.g / 2.0) {
      discard;
    }
  }
}
