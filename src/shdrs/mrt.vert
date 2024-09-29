#version 450 

layout(set = 0, binding = 0) uniform UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out  vec2     fragUV;
layout(location = 1) out  vec3     pos;
layout(location = 2) out vec3 normal;

vec2 stepPos = vec2(1.0/513.0); 
float stepUV = 1.0 / 513.0;

float rand(in vec2 uv) {
    return fract(sin(dot(uv,  vec2(12.9898,78.233))) * 43758.5453123);
}

float noise(in vec2 pos) {
    vec2 localPos = fract(pos);
    float a        = rand(floor(pos));
    float b        = rand(floor(pos + vec2(1.0, 0.0)));
    float c        = rand(floor(pos + vec2(1.0, 1.0)));
    float d        = rand(floor(pos + vec2(0.0, 1.0)));
    
    vec2 xy = localPos*localPos*(3.0 - 2.0*localPos);
    
    return mix(mix(a, b, xy.x), mix(d, c, xy.x), xy.y);
    
}

float fbm(in vec2 pos) {
   float ret  = 0.0;
   float freq = 1.0;
   
   ret += noise(pos);
   ret += 0.5     * noise(2.0 * pos);
   ret += 0.25    * noise(4.0 * pos);
   ret += 0.125   * noise(8.0 * pos);
   ret += 0.0625  * noise(16.0* pos);
   ret += 0.03125 * noise(32.0  * pos);
   
   return ret;
}

float n(in vec2 x) {
    return fbm(x*5.0);
    //return fbm(vec2(fbm(x*2.0), fbm(x*3.0)));
}

void main() {
    float c            = 0.10;
    vec4 worldPos      =  ubo.view * ubo.model * vec4(inPosition,1.0);
    pos  = worldPos.xyz;
    worldPos.y        +=  c * n(inUV);
    pos  = pos - worldPos.xyz;
    gl_Position        =  ubo.proj * worldPos;
    fragUV             =  inUV; 

    vec3 right       = inPosition; right.x += stepPos.x;
    vec3 up          = inPosition; up.y    += stepPos.y;
    
    right = (ubo.view * ubo.model * vec4(right,1.0)).xyz;
    up    = (ubo.view * ubo.model * vec4(up, 1.0)).xyz;

    right.y += c * n(vec2(inUV.x + stepUV, inUV.y));
    up.y    += c * n(vec2(inUV.x, inUV.y + stepUV));

    normal           = normalize( cross(right,up) );
}


