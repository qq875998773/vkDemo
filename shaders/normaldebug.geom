#version 450

layout (triangles) in; // in表示输入 triangles表示接受输入的类型为三角形
layout (line_strip, max_vertices = 2) out; // line_strip表示类型为连线，max_vertices=2表示这个连线最多允许两个顶点

layout (binding = 0) uniform UniformBufferObject 
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 testlight;
} ubo;

layout (location = 0) in vec3 inNormal[];

layout (location = 0) out vec3 outColor;

void main(void)
{
	float normalLength = 0.1; // 法线的长度系数
	vec3 pos = vec3(0.0, 0.0, 0.0); // 初始顶点位置为原点
	vec3 normal = vec3(0.0, 0.0, 0.0); // 初始法线方向为0
    for(int i=0; i<gl_in.length(); i++)
    { // 因为接收的是三角形所以这里会循环三次
    	pos += gl_in[i].gl_Position.xyz;
    	normal += inNormal[i].xyz;
    }
    
	pos = pos / 3.0; // 求三角形中点
	normal = normalize(normal / 3.0); // 求三个法线平均方向
	
	// 线的第一个点
    gl_Position = ubo.proj * vec4(pos, 1.0);
    outColor = vec3(1.0, 0.0, 0.0);
    EmitVertex();
    
	// 线的第二个点
    gl_Position = ubo.proj * vec4(pos - normal * normalLength, 1.0);
    outColor = vec3(0.0, 0.0, 1.0);
    EmitVertex();
    
	EndPrimitive();
}