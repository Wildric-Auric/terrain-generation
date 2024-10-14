#version 450 

layout(set = 0, binding = 0) uniform UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out  vec2  outUV;

layout(location = 1) out UBO {
   mat4 view;
   mat4 model;
   mat4 proj;
} outUBO;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    outUV       = inUV;

    outUBO.view  = ubo.view;
    outUBO.model = ubo.model;
    outUBO.proj  = ubo.proj;
}


