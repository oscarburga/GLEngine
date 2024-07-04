#pragma once

#include "glad/glad.h"
#include <filesystem>
#include "glm/glm.hpp"
#include "Render/GlShader.h"
#include "Render/GlRenderStructs.h"
#include "Render/GlIdTypes.h"

struct STextureAsset
{
	std::string Name;
	SGlTextureId Id;
};

struct SMeshAsset
{
	std::string Name;
	std::vector<SGeoSurface> Surfaces;
	SGPUMeshBuffers MeshBuffers;
};

struct SGLTFMaterial
{
};

struct SLoadedGLTF
{
	std::unordered_map<std::string, std::shared_ptr<SMeshAsset>> Meshes;
	std::unordered_map<std::string, std::shared_ptr<SNode>> Nodes;
	std::unordered_map<std::string, std::shared_ptr<STextureAsset>> Textures;
	std::vector<SGlSamplerId> Samplers;
	std::vector<std::shared_ptr<SNode>> RootNodes;
	// TODO: Materials
	//std::unordered_map<std::string, std::shared_ptr<SGLTFMaterial>> Materials;
	//SGlBufferId MaterialDataBuffer

	~SLoadedGLTF() { ClearAll(); }
	void ClearAll();
};

/*
* Class responsible for all basic asset loading done. Meshes, shaders, textures, etc.
* No object management should be done from within this class, it is only for loading.
*/
class CAssetLoader
{
	static std::filesystem::path ContentRoot; // TODO: Read this from some sort of a config/lua file.
	inline static CAssetLoader* AssetLoader = nullptr;
    inline static char infoLog[1024] = {};
	std::unordered_map<std::string, std::shared_ptr<SLoadedGLTF>> SceneCache;
	CAssetLoader() { LoadDefaultAssets(); };
	~CAssetLoader();

public:
	SGlTextureId WhiteTexture;
	SGlTextureId ErrorTexture;
	static void Create();
	static void Destroy();
	inline static CAssetLoader* Get() { return AssetLoader; }

	void LoadDefaultAssets();

	static std::optional<std::vector<SMeshAsset>> LoadGLTFMeshes(std::filesystem::path filePath);
	std::shared_ptr<SLoadedGLTF> LoadGLTFScene(std::filesystem::path filePath);

	static std::optional<SGPUTexture> LoadTexture2DFromFile(std::filesystem::path const& texturePath);
	static std::optional<SGPUTexture> LoadTexture2DFromBuffer(void* buffer, int size);

	static std::optional<CGlShader> LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath);

private:

	static std::optional<unsigned int> LoadSingleShader(const std::filesystem::path& shaderPath, unsigned int shaderType);
	static SGlTextureId RegisterTexture2D(void* rawTexData, int width, int height, int channels);

    static bool CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath);
};

