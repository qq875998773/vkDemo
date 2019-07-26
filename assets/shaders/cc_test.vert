//#version 450
//#extension GL_ARB_separate_shader_objects : enable

//layout(binding = 0) uniform UniformBufferObject {
//    mat4 model;
//    mat4 view;
//    mat4 proj;
//	vec3 testlight;
//} ubo;
//
//layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inColor;
//layout(location = 2) in vec2 inTexCoord;
//layout(location = 3) in vec3 inNormal;
//
//layout(location = 0) out vec3 fragColor;
//layout(location = 1) out vec2 fragTexCoord;
//layout(location = 2) out vec3 fragNormal; // 把法线传给fragment
//layout(location = 3) out vec3 fragPos; // 把模型顶点传给fragment
//layout(location = 4) out vec3 fragLight; // 把灯源位置传给fragment
//
//out gl_PerVertex {
//    vec4 gl_Position;
//};
//
//void main() {
//    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
//    fragColor = inColor;
//    fragTexCoord = inTexCoord;
//	fragNormal = inNormal;
//	fragPos = vec3(ubo.model * vec4(inPosition, 1.0));
//	fragLight = ubo.testlight;
//}



#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform SceneUBO
{
    mat4 view;
    mat4 projection;
    vec4 camera_position;
} scene_ubo;

layout(set = 0, binding = 1) uniform ModelUBO
{
    mat4 model;
    mat4 normal;
} model_ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coord;

layout(location = 0) out vec3 camera_position;

layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = scene_ubo.projection * scene_ubo.view * model_ubo.model * vec4(position, 1.0);
    camera_position = vec3(scene_ubo.camera_position);
    fragTexCoord = tex_coord;

}
