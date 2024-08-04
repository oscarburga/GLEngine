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

out VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
	vec4 Color;
	vec4 FragPosSunSpace;
	mat3 TBN;
} fs;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	gl_Position = sceneData.ViewProj * Model * vec4(vertex.Position, 1.0);
	fs.FragPos = (Model * vec4(vertex.Position, 1.0)).xyz;
	// Inneficient, ideally pass this as a uniform.
	fs.Normal = mat3(transpose(inverse(Model))) * vertex.Normal;
	fs.TexCoords = vec2(vertex.uv_x, vertex.uv_y);
	fs.Color = vertex.Color;

	vec3 T = normalize((Model * vec4(vertex.Tangent.xyz, 0)).xyz); 
	vec3 N = normalize((Model * vec4(vertex.Normal, 0)).xyz);
	T = normalize(T - dot(T, N) * N); // reorthogonalize
	vec3 B = cross(N, T) * vertex.Tangent.w; // w is gltf handedness
	fs.TBN = mat3(T, B, N);

	fs.FragPosSunSpace = sceneData.LightSpaceTransform * vec4(fs.FragPos, 1.0f);
}
