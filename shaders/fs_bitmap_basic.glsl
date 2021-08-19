#version 330 core

// [local_matrix, current_matrixf]
uniform mat4 matrixs[2];
uniform vec2 bounds[2];

// texture channel
uniform sampler2D ourTexture;

in vec3 TexCoord;

in vec3 vPos;

out vec4 FragColor;

vec2 calculate_uv() {
  vec4 mappedLT = matrixs[1] * matrixs[0] * vec4(bounds[0], 0.0, 1.0);
  vec4 mappedBR = matrixs[1] * matrixs[0] * vec4(bounds[1], 0.0, 1.0);

  vec4 mappedPos = matrixs[1] * vec4(vPos.xy, 0.0, 1.0);

  float totalX = mappedBR.x - mappedLT.x;
  float totalY = mappedBR.y - mappedLT.y;

  float vX = vPos.x / totalX;
  float vY = vPos.y / totalY;
  return vec2(vX, 1.0 - vY);
}

void main() {
  vec2 uv = calculate_uv();

  if (uv.x < 0 || uv.x > 1.0) {
    // discard;
  }

  if (uv.y < 0 || uv.y > 1.0) {
    // discard;
  }

  // vec4 tColor = texture(ourTexture, uv);

  vec4 tColor = vec4(uv, 0.0, 1.0);

  // if (vPos.z < 1.0) {
  // tColor = tColor * vPos.z;
  // }

  FragColor = tColor;
}