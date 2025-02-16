#pragma once

#include "glad/glad.h"
#include <filesystem>
#include "glm/glm.hpp"
#include "Render/GlShader.h"
#include "Render/GlRenderStructs.h"

struct SShaderLoadArgs;

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

public:
	std::shared_ptr<SLoadedGLTF> LoadGLTFScene(const std::filesystem::path& filePath);

	static std::optional<SGlTextureId> LoadTexture2DFromFile(std::filesystem::path const& texturePath, bool bFlipVertical);
	static std::optional<SGlTextureId> LoadTexture2DFromBuffer(void* buffer, size_t size, bool bFlipVertical);

	static std::optional<CGlShader> LoadShaderProgram(const SShaderLoadArgs& vsArgs, const SShaderLoadArgs& fsPath);
	static std::optional<CGlShader> LoadShaderProgram(const SShaderLoadArgs& vsArgs, const SShaderLoadArgs& gsPath, const SShaderLoadArgs& fsPath);

	static std::optional<std::string> ReadContentFileToString(const std::filesystem::path& filePath);
	static std::optional<std::string> ReadFileToString(const std::filesystem::path& rawFilePath);
private:

	static std::optional<unsigned int> LoadSingleShader(const SShaderLoadArgs& shaderArgs, unsigned int shaderType);
	static SGlTextureId RegisterTexture2D(void* rawTexData, int width, int height, int channels);

    static bool CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath);
};

// Warning: Args are CASE-SENSITIVE!!
struct SShaderLoadArgs
{
	template<typename StrLike>
	constexpr SShaderLoadArgs(StrLike&& path) : Path(path) {}

	template<typename StrLike, typename UMapInitializable>
	constexpr SShaderLoadArgs(StrLike&& path, UMapInitializable&& argsMap) : Path(path), Args(argsMap) {}

	template<typename StrLike>
	constexpr SShaderLoadArgs(StrLike&& path, std::initializer_list<std::pair<const std::string, std::string>>&& argsList) : Path(path), Args(argsList) {}

	std::filesystem::path Path; // Shader path
	std::unordered_map<std::string, std::string> Args; // ShaderArgs

	template<typename StrLike1, typename StrLike2>
	SShaderLoadArgs& SetArg(StrLike1&& ArgName, StrLike2&& Value)
	{
		Args[ArgName] = Value;
		return *this;
	}

	std::string ApplyToCode(std::string&& shaderCode) const;
};
