#version 450 
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

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

layout(set = 0, binding = 1) uniform sampler2D heightMap;

float n(in vec2 x) {
    return -6.0*texture(heightMap, x / 3.0).x;
    //return -.0*fbm(x*5.0);
    //return -5.0 * pow(fbm(vec2(fbm(x*1.5), fbm(x*3.0))), 0.9);
}
//---------------------------------

layout(location = 0) in  vec2 uv[];

layout(location = 1) in UBO { 
    mat4 view;
    mat4 model;
    mat4 projection;
} ubo[];  

layout(location = 0) out  vec2     fragUV;
layout(location = 1) out  vec3     worldPos;
layout(location = 2) out  vec3 normal;

float ccc    = pow(2.0, 9.0) + 1.0;
//vec2 stepPos = vec2(1.0/ccc); 
vec2  stepPos = vec2(0.1); 
float stepUV  = 0.01;

vec3 right(in vec3 p, in vec2 iuv, in mat4 model) {
   vec3 pos = p + vec3(stepPos.x*2.0, 0.0, 0.0); 
   vec2 st  = iuv + vec2(stepUV*2.0, 0.0);
   pos = (model * vec4(pos, 1.0)).xyz;
   pos.y += n(st);
   return pos;
}

vec3 up(in vec3 p, in vec2 iuv, in mat4 model) {
   vec3 pos = p + vec3(0.0, stepPos.y*2.0, 0.0); 
   vec2 st  = iuv + vec2(0.0,stepUV);
   pos = (model * vec4(pos, 1.0)).xyz;
   pos.y += n(st);
   return pos;
}

void main() {
    float c = 1.0;
    vec3 r; vec3 u;

    vec4 pos; vec4 pos1; vec4 pos2;
    pos    = ubo[0].model * gl_in[0].gl_Position;
    pos.y +=  n(uv[0]);

    pos1    = ubo[1].model * gl_in[1].gl_Position;
    pos1.y += n(uv[1]);

    pos2    = ubo[2].model * gl_in[2].gl_Position;
    pos2.y += n(uv[2]);
    
    fragUV      = uv[0];

//    normal      = normalize(-cross(pos1.xyz - pos.xyz, pos2.xyz - pos.xyz));
//    normal.x    = -normal.x;
//    normal.z    = -normal.z;
    r = right(gl_in[0].gl_Position.xyz, uv[0], ubo[0].model) - pos.xyz;
    u = up(gl_in[0].gl_Position.xyz, uv[0], ubo[0].model)    - pos.xyz;
    normal      = normalize(-cross(r,u));
    normal.x    = -normal.x;
    normal.z    = -normal.z;
    worldPos    = pos.xyz;
    pos         = ubo[0].projection * ubo[0].view * pos;
    gl_Position = pos;
    EmitVertex();

    fragUV      = uv[1];
    //normal      = normalize(-cross(pos1.xyz - pos.xyz, pos2.xyz - pos.xyz));
    r = right(gl_in[1].gl_Position.xyz, uv[1], ubo[1].model) - pos1.xyz;
    u = up(gl_in[1].gl_Position.xyz, uv[1], ubo[1].model)    - pos1.xyz;
    normal      = normalize(-cross(r,u));
    normal.x    = -normal.x;
    normal.z    = -normal.z;
    worldPos    = pos1.xyz;
    pos1        = ubo[1].projection * ubo[1].view * pos1;
    gl_Position = pos1;
    EmitVertex();

    fragUV      = uv[2];
    r = right(gl_in[2].gl_Position.xyz, uv[2], ubo[2].model) - pos2.xyz;
    u = up(gl_in[2].gl_Position.xyz, uv[2], ubo[2].model)    - pos2.xyz;
    normal      = normalize(-cross(r,u));
    normal.x    = -normal.x;
    normal.z    = -normal.z;
    worldPos    = pos2.xyz;
    pos2        = ubo[2].projection * ubo[2].view * pos2;
    gl_Position = pos2;
    EmitVertex();
}
