#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <format>
#include <fstream>
#include "GlDrawCommands.h"

#include "Materials.h"
#include "RenderObject.h"

SDrawObjectGpuData::SDrawObjectGpuData(const SRenderObject& render) :
	RenderTransform(render.RenderTransform),
	HasJoints(render.JointMatricesBuffer && render.VertexJointsDataBuffer),
	JointMatricesBaseIndex((uint32_t)render.JointMatricesBuffer.GetHeadInElems()),
	MaterialIndex((uint32_t)render.Material->DataBuffer.GetHeadInElems()),
	BonesIndexOffset(int32_t(render.VertexJointsDataBuffer.GetHeadInElems<int64_t>() - render.VertexBuffer.GetHeadInElems<int64_t>()))
{};
