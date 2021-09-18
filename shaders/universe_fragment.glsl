#version 330 core

// these macros are same as GLVertex2::VertexType
#define VERTEX_TYPE_NONE 0.0
#define VERTEX_TYPE_LINE_EDGE 1.0
#define VERTEX_TYPE_LINE_CAP 2.0
#define VERTEX_TYPE_LINE_BEVEL_JOIN 3.0
#define VERTEX_TYPE_LINE_ROUND_JOIN 4.0
#define VERTEX_TYPE_QUAD_IN 5.0
#define VERTEX_TYPE_QUAD_OUT 6.0

// these macros are same as GLShader::UniversealShader::Type
#define USER_FRAGMENT_TYPE_STENCIL 0

uniform vec4 UserColor;
uniform ivec4 UserData1;


// [mix, u, v]
in vec3 vPosInfo;

// final fragment color
out vec4 FragColor;

// Determin UserInput color
// this can be:
//  1. pure color
//  2. gradient color
//  3. texture color
vec4 CalculateUserColor() {
  // TODO implement other type color
  return vec4(UserColor.xyz * UserColor.w, UserColor.w);
}


// line edge aa alpha
float CalculateLineEdgeAlpha(float y) {
  float feathe = y;
  if (abs(feathe) < 0.9) {
    return 1.0;
  }

  float alpha = 1.0 - abs(feathe);
  alpha /= 0.1;
  return alpha;
}
// line cap alpha
float CalculateLineCapAlpha() {
  float alpha = vPosInfo.y;
  return alpha * CalculateLineEdgeAlpha(vPosInfo.z);
}

// Determin fragment alpha
// this may generate alpha gradient if needs anti-alias
float CalculateFragmentAlpha() {

  if (vPosInfo.x == VERTEX_TYPE_LINE_EDGE) {
    return CalculateLineEdgeAlpha(vPosInfo.y);
  }
  if (vPosInfo.x == VERTEX_TYPE_LINE_CAP) {
    return CalculateLineCapAlpha();
  }
  return 1.0;
}

void main() {
  // stencil, no need to calculate color
  if (UserData1.x == USER_FRAGMENT_TYPE_STENCIL) {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    return;
  }

  vec4 finalColor = CalculateUserColor();
  float finalAlpha = CalculateFragmentAlpha();

  FragColor = finalColor * finalAlpha;
}