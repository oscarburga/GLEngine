#include "GlShader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include "GlRenderStructs.h"

void SSolidMaterial::SetUniforms(CGlShader& shader)
{
    shader.SetUniform("material.ambient", Ambient);
    shader.SetUniform("material.diffuse", Diffuse);
    shader.SetUniform("material.specular", Specular);
    shader.SetUniform("material.shininess", Shininess);
}

void SMeshNode::Draw(const glm::mat4& topMatrix)
{
	SNode::Draw(topMatrix);
}

void SLoadedGLTF::ClearAll()
{
    for (auto& [name, meshPtr] : Meshes)
    {
        if (meshPtr->MeshBuffers.IndexBuffer)
            glDeleteBuffers(1, &*meshPtr->MeshBuffers.IndexBuffer);
        if (meshPtr->MeshBuffers.VertexBuffer)
            glDeleteBuffers(1, &*meshPtr->MeshBuffers.VertexBuffer);
    }
    Meshes.clear();

    for (auto& [name, texPtr] : Textures)
    {
        if (texPtr->Id)
            glDeleteTextures(1, &*texPtr->Id);
    }
    Textures.clear();

    static_assert(sizeof(SGlSamplerId) == sizeof(uint32_t));
    glDeleteSamplers((GLsizei)Samplers.size(), reinterpret_cast<uint32_t*>(Samplers.data()));
    Samplers.clear();
}

