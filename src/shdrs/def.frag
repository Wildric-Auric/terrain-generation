#version 450

layout(set = 0, binding = 0) uniform sampler2D colAtt;
layout(set = 0, binding = 1) uniform sampler2D normalAtt;
layout(set = 0, binding = 2) uniform sampler2D posAtt;
layout(set = 0, binding = 3) uniform LightData {
    vec3 pos;
    vec3 col;
} lightData;

layout(location = 0) in  vec2 uv;
layout(location = 0) out vec4 final;


void main() {
    
    vec4 color4 = texture(colAtt, uv);
    vec3 color  = color4.xyz;
 
//    if (color4.w == 0.0f) {
//        final = vec4(color,1.0);
//        return;
//    };

    vec3 normal = texture(normalAtt, uv).xyz;
    vec3 pos    = texture(posAtt, uv).xyz;

    //Diffusif light
    float d   = max(0.0,dot(normalize(lightData.pos - pos), normal));
    vec3  col = d * color;

    final = vec4(col,1.0);

    //debug
    //final = vec4((texture(colAtt,uv) + texture(normalAtt,uv) + texture(posAtt, uv)).xyz / 3.0, 1.0);
}
