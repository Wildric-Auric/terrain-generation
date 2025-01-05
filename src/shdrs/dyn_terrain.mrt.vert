#version 450

layout(set = 0, binding = 0) uniform UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out  vec2  outUV;
layout(location = 1) out  vec4  outPos;

void main() {
    vec4 hpos   = vec4(inPosition, 1.0);
    gl_Position = vec4(ubo.proj * ubo.view * ubo.model * hpos);
    outUV       = inUV;
    outPos      = gl_Position;
}
