#version 330 core

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec2 aTexCoord;
// layout (location = 1) in vec3 aColor; 
// layout (location = 2) in vec2 aTexCoord;

out vec2 texCoord;

uniform mat4 localToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToPerspective;

void main()
{
	gl_Position = cameraToPerspective * worldToCamera * localToWorld * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	texCoord = aTexCoord;
}