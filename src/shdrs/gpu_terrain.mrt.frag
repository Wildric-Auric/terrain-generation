#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 posAtt;


void main() {
    vec3  base    = vec3(0.3,0.6,0.1);;

    if (pos.y < -3.9) 
        base = vec3(0.7);
    else if ( normal.y < 0.75 ) {
        base    = vec3(0.3,0.2,0.1);
    }

    colAtt        = vec4(base, 1.0);
    normalAtt     = vec4(normal,1.0);
    posAtt        = vec4(pos,1.0);
}
