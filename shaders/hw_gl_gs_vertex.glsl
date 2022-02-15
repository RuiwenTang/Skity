
// [x, y]
layout(location = 0) in vec2 aPos;
// [mix, u, v]
layout(location = 1) in vec3 aPosInfo;

out vec2 vPos;
out vec3 vPosInfo;

void main() {
  vPos = aPos;
  vPosInfo = aPosInfo;
  gl_Position = vec4(aPos, 0.0, 1.0);
}
