#version 460 core
#extension GL_EXT_scalar_block_layout : require

struct SVertex {
	vec3 Position;
	float uv_x;
	vec3 Normal;
	float uv_y;
	vec4 Color;
};

// UBO
layout (binding = 0, std430) uniform SceneData {
	vec4 CameraPos;
	vec4 AmbientColor;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
} sceneData;

// SSBO
layout (binding = 0, std430) readonly buffer VertexBuffer {
	SVertex vertices[];
};

uniform mat4 Model;

out vec3 fsNormal;
out vec3 fsFragPos;
out vec2 fsTexCoords;
out vec4 fsColor;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	gl_Position = sceneData.ViewProj * Model * vec4(vertex.Position, 1.0);
	fsFragPos = (Model * vec4(vertex.Position, 1.0)).xyz;
	// Inneficient, ideally pass this as a uniform.
	fsNormal = mat3(transpose(inverse(Model))) * vertex.Normal;
	fsTexCoords = vec2(vertex.uv_x, vertex.uv_y);
	fsColor = vertex.Color;
}
