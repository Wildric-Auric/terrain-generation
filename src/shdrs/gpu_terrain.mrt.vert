#version 450 
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
    //return fbm(x*1.0);
    return fbm(vec2(fbm(x*2.0), fbm(x*3.0)));
}
//---------------------------------

layout(set = 0, binding = 0) uniform UBO { 
    mat4 view;
    mat4 model;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out  vec2  outUV;

layout(location = 1) out UBO {
   mat4 view;
   mat4 model;
   mat4 proj;
} outUBO;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    outUV       = inUV;

    outUBO.view  = ubo.view;
    outUBO.model = ubo.model;
    outUBO.proj  = ubo.proj;
}


