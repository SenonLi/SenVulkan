/*#version 330
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(position.x, -position.y, 0.0, 1.0);
    fragColor = color;
}//*/

#version 450
#extension GL_ARB_separate_shader_objects : enable
/*
	uniform values in shaders, are globals similar to dynamic state variables;
	can be changed at drawing time to alter the behavior of your shaders without having to recreate them.
*/
const int m_UniformBuffer_DS_BindingIndex = 1;
layout(binding = m_UniformBuffer_DS_BindingIndex) uniform MvpUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 projection;
} mvpUbo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = mvpUbo.projection * mvpUbo.view * mvpUbo.model * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}