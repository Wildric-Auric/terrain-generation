#version 450

layout(set = 0, binding = 0) uniform sampler2D colAtt;
layout(set = 0, binding = 1) uniform sampler2D normalAtt;
layout(set = 0, binding = 2) uniform sampler2D reflectionAtt;
layout(location = 0) in  vec2 uv;
layout(location = 0) out vec4 final;

void main() {
    //final = vec4((texture(colAtt,uv) + texture(normalAtt,uv) + texture(reflectionAtt, uv)).xyz / 3.0, 1.0);
    final = vec4(mix(texture(colAtt,uv).xyz, texture(reflectionAtt,uv).xyz, 0.4),1.0);
}
