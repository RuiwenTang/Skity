
layout(triangles) in;
// 32 * 3 = 96
layout(triangle_strip, max_vertices = 96) out;

uniform mat4 mvp;
uniform mat4 UserTransform;

// stroke width or circle radius
uniform float StrokeWidth;

// input from vertex shader
in vec3 vPos[];
// output to fragment shader
out vec3 fPos;

#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4
#define VERTEX_TYPE_QUAD_STROKE 6

#define MAX_QUAD_STEP 32

vec2 quad_bezier(float u, vec2 p0, vec2 p1, vec2 p2) {
  float b0 = (1.0 - u) * (1.0 - u);
  float b1 = 2.0 * u * (1.0 - u);
  float b2 = u * u;

  vec2 p = b0 * p0 + b1 * p1 + b2 * p2;
  return p;
}

void generate_normal_triangle() {
  gl_Position = mvp * UserTransform * vec4(vPos[0].xy, 0.0, 1.0);
  fPos = vPos[0];
  EmitVertex();

  gl_Position = mvp * UserTransform * vec4(vPos[1].xy, 0.0, 1.0);
  fPos = vPos[1];
  EmitVertex();

  gl_Position = mvp * UserTransform * vec4(vPos[2].xy, 0.0, 1.0);
  fPos = vPos[2];
  EmitVertex();
}

void generate_bezier(int quad_in) {
  vec2 p0 = gl_in[0].gl_Position.xy;
  vec2 p1 = gl_in[1].gl_Position.xy;
  vec2 p2 = gl_in[2].gl_Position.xy;

  float step = 1.0 / float(MAX_QUAD_STEP - 1);
  float u = 0.0;
  vec2 points[MAX_QUAD_STEP];

  for (int i = 0; i < MAX_QUAD_STEP; i++) {
    points[i] = quad_bezier(u, p0, p1, p2);
    u += step;
  }

  for (int i = 0; i < 31; i++) {
    if (quad_in == 1) {
      gl_Position = mvp * UserTransform * vec4(p0, 0.0, 1.0);
      fPos = vec3(p0, vPos[0].z);
    } else {
      gl_Position = mvp * UserTransform * vec4(p1, 0.0, 1.0);
      fPos = vec3(p1, vPos[0].z);
    }
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i], 0.0, 1.0);
    fPos = vec3(points[i], vPos[0].z);
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i + 1], 0.0, 1.0);
    fPos = vec3(points[i + 1], vPos[0].z);
    EmitVertex();

    EndPrimitive();
  }
}

void generate_bezier_stroke() {
  vec2 p0 = gl_in[0].gl_Position.xy;
  vec2 p1 = gl_in[1].gl_Position.xy;
  vec2 p2 = gl_in[2].gl_Position.xy;

  vec2 p10 = p1 - p0;
  vec2 p21 = p2 - p1;

  float step = 1.0 / float(MAX_QUAD_STEP - 1);
  float u = 0.0;
  for (int i = 0; i < MAX_QUAD_STEP; i++) {
    vec2 p = quad_bezier(u, p0, p1, p2);

    vec2 a = p0 + u * p10;
    vec2 b = p1 + u * p21;

    vec2 d = normalize(b - a);
    vec2 n = vec2(d.y, -d.x);

    vec2 up = p + n * StrokeWidth;
    vec2 dp = p - n * StrokeWidth;

    gl_Position = mvp * vec4(up, 0.0, 1.0);
    fPos = vec3(up, vPos[0].z);
    EmitVertex();

    gl_Position = mvp * vec4(dp, 0.0, 1.0);
    fPos = vec3(dp, vPos[0].z);
    EmitVertex();

    u += step;
  }
}

void main() {
  int type = int(vPos[0].z);

  if (type == VERTEX_TYPE_QUAD_IN) {
    generate_bezier(1);
  } else if (type == VERTEX_TYPE_QUAD_OUT) {
    generate_bezier(0);
  } else if (type == VERTEX_TYPE_QUAD_STROKE) {
    generate_bezier_stroke();
  } else {
    generate_normal_triangle();
  }
}