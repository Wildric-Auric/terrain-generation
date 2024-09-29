#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 reflectionAtt;

vec3 lightPos = vec3(1.0, 0.5, 100.0);

void main() {

    float m = 0.15;
    //colAtt        = vec4(vec3(pos.y*pos.y / m),1.0);
    //Diffusif light
    float d       = max(0.0,dot(normalize(lightPos - pos), normal));
    float col     = d + 0.1;
    colAtt        = vec4(vec3(col), 1.0);
    normalAtt     = vec4(normal,1.0);
    reflectionAtt = vec4(vec3(pos.y*pos.y / m),1.0);
}
