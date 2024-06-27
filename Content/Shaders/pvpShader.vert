#version 460 core

// layout (location = 0) in vec3 aPos; 
// layout (location = 1) in vec3 aNormal; 
// layout (location = 2) in vec2 aTexCoords; 

struct SVertex {
	vec3 Position;
	float uv_x;
	vec3 Normal;
	float uv_y;
	vec4 Color;
};

uniform mat4 localToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToPerspective;

layout (binding = 0, std430) readonly buffer VertexBuffer {
	SVertex vertices[];
};

out vec3 fsNormal;
out vec3 fsFragPos;
out vec2 fsTexCoords;
out vec4 fsColor;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	gl_Position = cameraToPerspective * worldToCamera * localToWorld * vec4(vertex.Position, 1.0);
	fsFragPos = (localToWorld * vec4(vertex.Position, 1.0)).xyz;
	// Inneficient, ideally pass this as a uniform.
	fsNormal = mat3(transpose(inverse(localToWorld))) * vertex.Normal;
	fsTexCoords = vec2(vertex.uv_x, vertex.uv_y);
	fsColor = vertex.Color;
}
