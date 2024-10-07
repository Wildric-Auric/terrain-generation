#version 450

layout(location = 0) in  vec3 fragPos;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 posAtt;

vec3 lightPos = vec3(-0.4, .81, -0.2);

void main() {
    vec3 col      = vec3(0.7);
    colAtt        = vec4(col, 1.0);
    normalAtt     = vec4(col,1.0);
    posAtt        = vec4(fragPos,1.0);
}
