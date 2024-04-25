#version 460 core

layout (location = 0) in vec3 aPos; 
layout (location = 1) in vec3 aNormal; 
layout (location = 2) in vec2 aTexCoords; 

uniform mat4 localToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToPerspective;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
	gl_Position = cameraToPerspective * worldToCamera * localToWorld * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	FragPos = (localToWorld * vec4(aPos, 1.0)).xyz;
	// Inneficient, ideally pass this as a uniform.
	Normal = mat3(transpose(inverse(localToWorld))) * aNormal;
	TexCoords = aTexCoords;
}