#version 450
#extension GL_ARB_separate_shader_objects : enable

// Defined in head of SLVK_02_Texture: const int m_COMB_IMA_SAMPLER_DS_BindingIndex = 3;
// Have to input literal integer to binding layout of uniform sampler2D
layout(binding = 3) uniform sampler2DArray textureSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
	int textureArrayIndex = 4;
    outColor = texture(textureSampler, vec3(fragTexCoord, textureArrayIndex));
}