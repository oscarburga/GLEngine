#version 330 core

layout (location = 0) in vec3 aPos; // Input variable vec3 at location 0
layout (location = 1) in vec3 aColor; // Input variable vec3 at location 1
out vec4 vertexColor;

void main()
{
   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
   vertexColor = vec4(aColor.x, aColor.y, aColor.z, 1.0);
}