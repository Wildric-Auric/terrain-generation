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
    vec3 normal = texture(normalAtt, uv).xyz;
    vec3 pos    = texture(posAtt, uv).xyz;
    
    vec3 color  = color4.xyz;

    vec3 ldir = normalize(lightData.pos - pos);
    float d   = max(0.0, ldir.y);
    vec3 bg        = vec3(uv.y * d * (1.0 - d) + d*uv.y, d,  d);
    if (pos.z == 0.0) {
        final = vec4(bg,0.0);
        return;
    };

    //Diffusif light
    d   = max(0.0,dot(ldir, normal));
    vec3  col = d * color ; 

    float t = clamp(pos.z / lightData.col.x,0.0,1.0 );
    final = vec4(mix(bg*1.01,col, pow(t,32.0)),1.0);

    //debug
    //final = vec4((texture(colAtt,uv) + texture(normalAtt,uv) + texture(posAtt, uv)).xyz / 3.0, 1.0);
}
