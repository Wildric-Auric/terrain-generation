#version 450

layout(location = 0) out vec4 colAtt;
layout(location = 1) out vec4 normalAtt;
layout(location = 2) out vec4 reflectionAtt;

void main() {
	//colAtt        = vec4(12.0 / 255., 207./255., 184. / 255., 1.0);
    colAtt        = vec4(1.0, 0.0, 0.0,1.0);
    normalAtt     = vec4(0.0, 1.0, 0.0,1.0);
    reflectionAtt = vec4(0.0, 0.0, 1.0,1.0);
}
