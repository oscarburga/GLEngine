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

