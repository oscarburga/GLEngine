#version 460 core

struct SVertex {
	vec3 Position;
	float uv_x;
	vec3 Normal;
	float uv_y;
	vec4 Color;
	vec4 Tangent;
};

// UBO
layout (binding = 0, std140) uniform SceneData {
	vec4 CameraPos;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
	mat4 LightSpaceTransform;
} sceneData;

// SSBO
layout (binding = 0, std430) readonly buffer VertexBuffer {
	SVertex vertices[];
};

layout (location = 0) uniform mat4 Model;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	gl_Position = sceneData.LightSpaceTransform * Model * vec4(vertex.Position, 1.0);
}
