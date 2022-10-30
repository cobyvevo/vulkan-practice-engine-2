#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uvCoord;

layout(set = 1, binding = 0) uniform sampler2D tex;

void main() {
	//outColor = (vec4(uvCoord,0.5,1.0) * 0.5) + (vec4(color,1.0)*0.5); //usually ambient isnt here
	outColor = vec4(fragColor,1.0);
}