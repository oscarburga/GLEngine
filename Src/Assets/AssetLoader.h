#pragma once

#include "glad/glad.h"
#include <filesystem>
#include "glm/glm.hpp"
#include "Render/GlShader.h"
#include "Render/GlRenderStructs.h"

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
	~CAssetLoader();

public:
	SGlTextureId WhiteTexture;
	SGlTextureId ErrorTexture;
	std::shared_ptr<SPbrMaterial> WhiteMaterial;
	std::shared_ptr<SPbrMaterial> ErrorMaterial;
	std::shared_ptr<SMeshNode> AxisMesh; // Probably move this one to GlRenderer?
	static void Create();
	static void Destroy();
	inline static CAssetLoader* Get() { return AssetLoader; }

	void LoadDefaultAssets();

private:
	static std::optional<std::vector<SMeshAsset>> LoadGLTFMeshes(std::filesystem::path filePath);
public:
	std::shared_ptr<SLoadedGLTF> LoadGLTFScene(const std::filesystem::path& filePath);

	// TODO: these return SGlTextureId
	static std::optional<SGlTextureId> LoadTexture2DFromFile(std::filesystem::path const& texturePath, bool bFlipVertical);
	static std::optional<SGlTextureId> LoadTexture2DFromBuffer(void* buffer, int size, bool bFlipVertical);

	static std::optional<CGlShader> LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath);

private:

	static std::optional<unsigned int> LoadSingleShader(const std::filesystem::path& shaderPath, unsigned int shaderType);
	static SGlTextureId RegisterTexture2D(void* rawTexData, int width, int height, int channels);

    static bool CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath);
};

