#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (location = 5) uniform sampler2DArray screenTexture;
layout (location = 34) uniform int texIndex;

void main()
{
	FragColor = texture(screenTexture, vec3(TexCoords, texIndex));
}
