#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 mvp;

void main() { gl_Position = mvp * vec4(aPos, 0.0, 1.0); }