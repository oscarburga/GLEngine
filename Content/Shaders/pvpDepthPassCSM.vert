#version 460 core

#define COMPILEARG_BEGIN
#define NumCascades 3
#define MAX_CASCADES 16
#define MAX_JOINTS 200
#define MAX_DRAWS 200
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

struct SDrawObjectData {
	mat4 RenderTransform;
	bool bHasJoints;
	int JointMatricesBaseIndex;
	int MaterialIndex;
	int BonesIndexOffset;
};

layout (binding = 3, std140) uniform DrawDataUBO {
	SDrawObjectData DrawObjectData[MAX_DRAWS];
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
// layout (location = 10) uniform int BaseDrawId;

void main()
{
	const SVertex vertex = vertices[gl_VertexID];
	SDrawObjectData drawData = DrawObjectData[gl_BaseInstance];
	mat4 toWorldTransMat = drawData.RenderTransform;
	if (drawData.bHasJoints) {
		SVertexJointData jointsData = vertexJointData[drawData.BonesIndexOffset + gl_VertexID];
		jointsData.Joints += uvec4(drawData.JointMatricesBaseIndex);

		toWorldTransMat *= 
			jointsData.Weights.x * jointMatrices[jointsData.Joints.x] +
			jointsData.Weights.y * jointMatrices[jointsData.Joints.y] +
			jointsData.Weights.z * jointMatrices[jointsData.Joints.z] +
			jointsData.Weights.w * jointMatrices[jointsData.Joints.w];
	}
	gl_Position = toWorldTransMat * vec4(vertex.Position, 1.0);
}
