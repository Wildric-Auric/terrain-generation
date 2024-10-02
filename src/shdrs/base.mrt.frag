#version 450

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 reflectionAtt;

vec3 lightPos = vec3(-0.4, .81, -0.2);

void main() {
    vec3 col      = vec3(0.7);
    colAtt        = vec4(col, 1.0);
    normalAtt     = vec4(col,1.0);
    reflectionAtt = vec4(col,1.0);
}