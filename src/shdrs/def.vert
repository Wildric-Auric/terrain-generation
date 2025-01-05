#version 450 

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 0) out vec2 uv;

layout(set = 0, binding = 4) uniform LightData {
    mat4 content;
} modelMatrix;

void main() {
    gl_Position   = modelMatrix.content * vec4(inPosition, 1.0);
    uv = inUV;
}
