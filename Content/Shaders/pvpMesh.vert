#version 460 core

#define COMPILEARG_BEGIN
#define MAX_CASCADES 16
#define MAX_JOINTS 200
#define COMPILEARG_END

struct SVertex {
	vec3 Position;
	float uv_x;
	vec3 Normal;
	float uv_y;
	vec4 Color;
	vec4 Tangent;
};

struct SVertexJointData {
	uvec4 Joints;
	vec4 Weights;
};

// UBO
layout (binding = 0, std140) uniform SceneData {
	vec4 CameraPos;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
	vec4 CascadeDistances[MAX_CASCADES];
	mat4 LightSpaceTransforms[MAX_CASCADES];
} sceneData;

layout (binding = 2, std140) uniform JointMatsUBO {
	mat4 jointMatrices[MAX_JOINTS];
};

// SSBO
layout (binding = 0, std430) readonly buffer VertexBuffer {
	SVertex vertices[];
};

layout (binding = 1, std430) readonly buffer VertexJointDataBuffer {
	SVertexJointData vertexJointData[];
};

layout (location = 0) uniform mat4 Model;
layout (location = 6) uniform bool bHasJoints;
layout (location = 7) uniform int BoneBufferOffset;
layout (location = 9) uniform int JointMatricesIndexOffset;

out VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
	vec4 Color;
	mat3 TBN;
} fs;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	mat4 toWorldTransMat = Model;
	if (bHasJoints) {
		SVertexJointData jointsData = vertexJointData[int(gl_VertexID) + BoneBufferOffset];
		jointsData.Joints += uvec4(JointMatricesIndexOffset);
		toWorldTransMat *= 
			jointsData.Weights.x * jointMatrices[jointsData.Joints.x] +
			jointsData.Weights.y * jointMatrices[jointsData.Joints.y] +
			jointsData.Weights.z * jointMatrices[jointsData.Joints.z] +
			jointsData.Weights.w * jointMatrices[jointsData.Joints.w];
	}
	gl_Position = sceneData.ViewProj * toWorldTransMat * vec4(vertex.Position, 1.0);
	fs.FragPos = (toWorldTransMat * vec4(vertex.Position, 1.0)).xyz;
	// Inneficient, ideally pass this as a uniform.
	fs.Normal = mat3(transpose(inverse(toWorldTransMat))) * vertex.Normal;
	fs.TexCoords = vec2(vertex.uv_x, vertex.uv_y);
	fs.Color = vertex.Color;

	vec3 T = normalize((toWorldTransMat * vec4(vertex.Tangent.xyz, 0)).xyz); 
	vec3 N = normalize((toWorldTransMat * vec4(vertex.Normal, 0)).xyz);
	T = normalize(T - dot(T, N) * N); // reorthogonalize
	vec3 B = cross(N, T) * vertex.Tangent.w; // w is gltf handedness
	fs.TBN = mat3(T, B, N);
}
