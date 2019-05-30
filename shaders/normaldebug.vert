#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 testlight;
} ubo;

layout (location = 0) in vec3 inPosition;
layout (location = 3) in vec3 inNormal;

layout (location = 0) out vec3 outNormal;

void main(void)
{
	outNormal = (ubo.view *(ubo.model * vec4(inNormal, 1.0))).xyz; // 把法线矩阵变换后传给geometry shader
	gl_Position = ubo.view *(ubo.model * vec4(inPosition, 1.0)); // 把顶点矩阵变换后传给geometry shader
}