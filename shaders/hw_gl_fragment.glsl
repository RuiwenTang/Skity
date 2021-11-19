#version 330 core

#define M_PI 3.1415926535897932384626433832795
// Fixme to solve uniform array length
#define MAX_COLORS 32

// image texture
uniform sampler2D UserTexture;
// font texture
uniform sampler2D FontTexture;

uniform vec4 UserColor;
uniform ivec4 UserData1;
uniform ivec4 UserData2;
uniform vec4 UserData3;
uniform vec4 UserData4;

// gradient color and stops
uniform vec4 GradientColors[MAX_COLORS];
uniform float GradientStops[MAX_COLORS];

// [x, y]
in vec2 vPos;
// [mix, u, v]
in vec3 vPosInfo;