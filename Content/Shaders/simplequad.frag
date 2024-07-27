#version 460 core

in vec2 TexCoords;
out vec4 FragColor;

layout (location = 5) uniform sampler2D screenTexture;

void main()
{
	FragColor = texture(screenTexture, TexCoords);
}
