#version 460 core

// SSBO
layout (binding = 0, std430) readonly buffer VertexBuffer {
	vec4 vertices[];
};

out vec2 TexCoords;

void main()
{
	vec4 vertex = vertices[gl_VertexID];
	gl_Position = vec4(vertex.xy, 0.f, 1.0);
	TexCoords = vertex.zw;
}
