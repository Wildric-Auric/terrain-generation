#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 pos;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 posAtt;


//--------------Noise Functions----------------------
float rand(in vec2 uv) {
    return fract(sin(dot(uv,  vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(in vec2 pos) {
    vec2 localPos = fract(pos);
    float a        = rand(floor(pos));
    float b        = rand(floor(pos + vec2(1.0, 0.0)));
    float c        = rand(floor(pos + vec2(1.0, 1.0)));
    float d        = rand(floor(pos + vec2(0.0, 1.0)));
    
    vec2 xy = smoothstep(vec2(0.0), vec2(1.0), localPos); //localPos*localPos*(3.0 - 2.0*localPos);
    
    return mix(mix(a, b, xy.x), mix(d, c, xy.x), xy.y);
}

float fbm(vec2 pos) {
   float res = 0.0;
   res =   3.0 * pow(noise(pos), 4.5);
   res +=  0.25  * pow(noise(pos * 2.0 ), 2.0);
   res +=  0.325 *  noise(pos    * 4.0 );
   res +=  0.0925 * noise(pos    * 16.0 );
   res +=  0.0512 * noise(pos    * 32.0 );

//   ret += noise(pos);
//   ret += 0.5     * noise(2.0 * pos);
//   ret += 0.25    * noise(4.0 * pos);
//   ret += 0.125   * noise(8.0 * pos);
//   ret += 0.0625  * noise(16.0* pos);
//   ret += 0.03125 * noise(32.0  * pos);
   
   return res;
}

void main() {
    vec3  base    = vec3(0.1 + 0.1 * fbm(pos.xz*3.0 + 100.0), 0.3 + 0.1*fbm(pos.xz*2.0),0.1);;

    if (pos.y < -3.0 - 2.5*normal.y) 
        base = vec3(0.7);
    else if ( normal.y < 0.75 ) {
        base    = vec3(0.3,0.2,0.1);
    }

    colAtt        = vec4(base, 1.0);
    normalAtt     = vec4(normal,1.0);
    posAtt        = vec4(pos,1.0);
}
