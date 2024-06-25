#version 460 core

layout (location = 0) in vec3 aPos; 

uniform mat4 localToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToPerspective;

void main()
{
	gl_Position = cameraToPerspective * worldToCamera * localToWorld * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}