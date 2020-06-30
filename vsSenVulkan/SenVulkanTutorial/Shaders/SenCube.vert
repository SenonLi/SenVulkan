#version 450
#extension GL_ARB_separate_shader_objects : enable
/*
	uniform values in shaders, are globals similar to dynamic state variables;
	can be changed at drawing time to alter the behavior of your shaders without having to recreate them.
*/
const int m_UniformBuffer_DS_BindingIndex = 0;
layout(binding = m_UniformBuffer_DS_BindingIndex) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position.x, -position.y, position.z, 1.0);
    fragTexCoord = inTexCoord;
}