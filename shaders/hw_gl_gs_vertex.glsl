
// [x, y, mix]
layout(location = 0) in vec3 aPos;

out vec3 vPos;

void main() {
  vPos = aPos;
  gl_Position = vec4(aPos.xy, 0.0, 1.0);
}
