#version 460 core

#define COMPILEARG_BEGIN
#define NumCascades 3
#define MAX_CASCADES 16
#define COMPILEARG_END

layout(triangles, invocations = NumCascades) in;
layout(triangle_strip, max_vertices = 3) out;

layout (binding = 0, std140) uniform SceneData {
	vec4 CameraPos;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
	mat4 LightSpaceTransform;
	mat4 LightSpaceTransforms[MAX_CASCADES];
} sceneData;

void main()
{
	gl_Layer = gl_InvocationID;
	for (int i = 0; i<3; ++i) {
		gl_Position = sceneData.LightSpaceTransforms[gl_InvocationID] * gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
	// gl_FragDepth = gl_FragCoord.z;
}
