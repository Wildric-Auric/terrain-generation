#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec4 pos;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 posAtt;


void main() {
    vec3  base    = vec3(0.5);
    colAtt        = vec4(base, 1.0);
    normalAtt     = vec4(vec3(0.0,-1.0,0.0),1.0);
    posAtt        = vec4(pos.xyz / pos.w,1.0);
}
