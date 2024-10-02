#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 reflectionAtt;

vec3 lightPos = vec3(-0.4, .81, -0.2);

void main() {

    float m = 0.15;
    //Diffusif light
    vec3  base    = vec3(0.3,0.2,0.1);
    float d       = max(0.0,dot(normalize(lightPos - pos), normal));
    vec3 col      = d * base;
    colAtt        = vec4(col, 1.0);
    normalAtt     = vec4(normal,1.0);
    reflectionAtt = vec4(vec3(pos.y*pos.y / m),1.0);
}
