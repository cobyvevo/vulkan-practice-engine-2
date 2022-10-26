#version 450

layout(location = 0) in vec3 vPos;
layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform CameraBuffer{
	mat4 viewproj;
} Camera;

vec2 positions[3] = vec2[](
	vec2(0,-0.3),
	vec2(0.25,0.34),
	vec2(-0.4,0.2)
);
/*
vec3 cols[24] = vec3[]( //jesus christ
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0),
	vec3(1.0, 0.0, 0.0),
	vec3(0.0, 1.0, 0.0),
	vec3(0.0, 0.0, 1.0),
	vec3(1.0, 1.0, 1.0)
);
*/
void main() {
	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);

	gl_Position = Camera.viewproj * vec4(vPos,1.0);
	fragColor = vPos;//cols[gl_VertexIndex];
}