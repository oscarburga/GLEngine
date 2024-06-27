#pragma once

#include <filesystem>
#include "glm/glm.hpp"
#include "Graphics/Shader.h"

struct SGPUMeshBuffers
{
	uint32_t IndexBuffer = 0;
	uint32_t VertexBuffer = 0;
};

struct SGeoSurface
{
	uint32_t StartIndex = 0;
	uint32_t Count = 0;
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

	static std::optional<CShader> LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath);

private:

	static std::optional<unsigned int> LoadSingleShader(const std::filesystem::path& shaderPath, unsigned int shaderType);

    inline static char infoLog[1024] = {};
    static bool CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath);
};



