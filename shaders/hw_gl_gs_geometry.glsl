
layout(triangles) in;
// 16 * 3 = 48
layout(triangle_strip, max_vertices = 48) out;

uniform mat4 mvp;
uniform mat4 UserTransform;

// stroke width or circle radius
uniform float StrokeWidth;

// input from vertex shader
in vec2 vPos[];
in vec3 vPosInfo[];
// output to fragment shader
out vec2 fPos;
out vec3 fPosInfo;

#define VERTEX_TYPE_LINE_NORMAL 1
#define VERTEX_TYPE_CIRCLE 2
#define VERTEX_TYPE_QUAD_IN 3
#define VERTEX_TYPE_QUAD_OUT 4
#define VERTEX_TYPE_QUAD_STROKE 6

#define MAX_QUAD_STEP 16

vec2 quad_bezier(float u, vec2 p0, vec2 p1, vec2 p2) {
  float b0 = (1.0 - u) * (1.0 - u);
  float b1 = 2.0 * u * (1.0 - u);
  float b2 = u * u;

  vec2 p = b0 * p0 + b1 * p1 + b2 * p2;
  return p;
}

vec2 quad_bezier_tangent(float u, vec2 p0, vec2 p1, vec2 p2) {
  vec2 p = 2.0 * (1.0 - u) * (p1 - p0) + 2.0 * u * (p2 - p1);

  return normalize(p);
}

void generate_circle() {
  vec2 center = vPosInfo[0].yz;

  vec2 p0 = gl_in[1].gl_Position.xy;
  vec2 p1 = gl_in[2].gl_Position.xy;

  float step = 1.0 / float(MAX_QUAD_STEP - 1);
  float u = 0.0;

  vec2 points[MAX_QUAD_STEP];

  for (int i = 0; i < MAX_QUAD_STEP; i++) {
    vec2 tp = mix(p0, p1, u);

    vec2 dir = normalize(tp - center);
    points[i] = center + dir * StrokeWidth * 0.5;

    u += step;
  }

  fPosInfo = vPosInfo[0];
  for (int i = 0; i < MAX_QUAD_STEP - 1; i++) {
    gl_Position = mvp * UserTransform * gl_in[0].gl_Position;
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i], 0.0, 1.0);
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i + 1], 0.0, 1.0);
    EmitVertex();

    EndPrimitive();
  }
}

void generate_normal_triangle() {
  gl_Position = mvp * UserTransform * vec4(vPos[0].xy, 0.0, 1.0);
  fPos = vPos[0];
  fPosInfo = vPosInfo[0];
  EmitVertex();

  gl_Position = mvp * UserTransform * vec4(vPos[1].xy, 0.0, 1.0);
  fPos = vPos[1];
  fPosInfo = vPosInfo[1];
  EmitVertex();

  gl_Position = mvp * UserTransform * vec4(vPos[2].xy, 0.0, 1.0);
  fPos = vPos[2];
  fPosInfo = vPosInfo[2];
  EmitVertex();
}

void generate_bezier(int quad_in) {
  vec2 p0 = gl_in[0].gl_Position.xy;
  vec2 p1 = gl_in[1].gl_Position.xy;
  vec2 p2 = gl_in[2].gl_Position.xy;

  float step = 1.0 / float(MAX_QUAD_STEP - 1);
  float u = 0.0;
  vec2 points[MAX_QUAD_STEP];

  for (int i = 0; i < MAX_QUAD_STEP - 1; i++) {
    points[i] = quad_bezier(u, p0, p1, p2);
    u += step;
  }

  points[MAX_QUAD_STEP - 1] = quad_bezier(1.0, p0, p1, p2);

  for (int i = 0; i < MAX_QUAD_STEP - 1; i++) {
    if (quad_in == 1) {
      gl_Position = mvp * UserTransform * vec4(p0, 0.0, 1.0);
      fPos = p0;
      fPosInfo = vPosInfo[0];
    } else {
      gl_Position = mvp * UserTransform * vec4(p1, 0.0, 1.0);
      fPos = p1;
      fPosInfo = vPosInfo[0];
    }
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i], 0.0, 1.0);
    fPos = points[i];
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(points[i + 1], 0.0, 1.0);
    fPos = points[i + 1];
    EmitVertex();

    EndPrimitive();
  }
}

void generate_bezier_stroke() {
  vec2 p0 = gl_in[0].gl_Position.xy;
  vec2 p1 = gl_in[1].gl_Position.xy;
  vec2 p2 = gl_in[2].gl_Position.xy;

  fPosInfo = vPosInfo[0];

  float step = 1.0 / float(MAX_QUAD_STEP - 1);
  float u = 0.0;
  for (int i = 0; i < MAX_QUAD_STEP; i++) {
    vec2 p = quad_bezier(u, p0, p1, p2);

    vec2 tangent = quad_bezier_tangent(u, p0, p1, p2);
    vec2 n = vec2(tangent.y, -tangent.x);

    vec2 up = p + n * StrokeWidth * 0.5;
    vec2 dp = p - n * StrokeWidth * 0.5;

    gl_Position = mvp * UserTransform * vec4(up, 0.0, 1.0);
    fPos = up;
    EmitVertex();

    gl_Position = mvp * UserTransform * vec4(dp, 0.0, 1.0);
    fPos = dp;
    EmitVertex();

    u += step;
  }
  EndPrimitive();
}

void main() {
  int type = int(vPosInfo[0].x);

  if (type == VERTEX_TYPE_CIRCLE) {
    generate_circle();
  } else if (type == VERTEX_TYPE_QUAD_IN) {
    generate_bezier(1);
  } else if (type == VERTEX_TYPE_QUAD_OUT) {
    generate_bezier(0);
  } else if (type == VERTEX_TYPE_QUAD_STROKE) {
    generate_bezier_stroke();
  } else {
    generate_normal_triangle();
  }
}