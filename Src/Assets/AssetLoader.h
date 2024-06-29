#pragma once

#include "glad/glad.h"
#include <filesystem>
#include "glm/glm.hpp"
#include "Graphics/Shader.h"

struct SGPUMeshBuffers
{
	uint32_t IndexBuffer = 0;
	uint32_t VertexBuffer = 0;
};

struct SGPUTexture
{
	uint32_t Texture = 0;
	uint32_t Sampler = 0;
};

struct SSolidMaterial
{
	glm::vec3 Ambient {};
	glm::vec3 Diffuse {};
	glm::vec3 Specular {};
	float Shininess = 32.f;
	void SetUniforms(CShader& shader);
};

struct STexturedMaterial
{
	SGPUTexture Diffuse = {};
	SGPUTexture Specular = {};
	float Shininess = 32.f;
};

struct SGeoSurface
{
	uint32_t StartIndex = 0;
	uint32_t Count = 0;
	// Temp: this eventually needs to be replaced by a templated material or a variant
	STexturedMaterial Material = {}; 
};

struct SMeshAsset
{
	std::string Name;
	std::vector<SGeoSurface> Surfaces;
	SGPUMeshBuffers MeshBuffers;
};

struct SVertex
{
	glm::vec3 Position;
	float uv_x;
	glm::vec3 Normal;
	float uv_y;
	glm::vec4 Color;
};

/*
* Class responsible for all basic asset loading done. Meshes, shaders, textures, etc.
* No object management should be done from within this class, it is only for loading.
*/
class CAssetLoader
{
public:

	/*
	* TODO: Read all of these from some sort of a config/lua file.
	*/
	static std::filesystem::path ContentRoot;
	// static std::filesystem::path MeshesDir;
	// static std::filesystem::path ScriptsDir;

	static std::optional<std::vector<SMeshAsset>> LoadGLTFMeshes(std::filesystem::path filePath);

	static std::optional<SGPUTexture> LoadTexture2DFromFile(std::filesystem::path const& texturePath);
	static std::optional<SGPUTexture> LoadTexture2DFromBuffer(void* buffer, int size);

	static std::optional<CShader> LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath);

private:

	static std::optional<unsigned int> LoadSingleShader(const std::filesystem::path& shaderPath, unsigned int shaderType);
	static void RegisterTexture2D(void* stbiTexData, SGPUTexture& gpuTex, int w, int h, int c);

    inline static char infoLog[1024] = {};
    static bool CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath);
};

