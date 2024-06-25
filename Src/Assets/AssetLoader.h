#pragma once

#include <filesystem>

/*
* Class responsible for all basic asset loading done. Meshes, shaders, textures, etc.
* No object management should be done from within this class, it is only for loading.
*/
class CAssetLoader
{
public:

private:
	/*
	* TODO: Read all of these from some sort of a config/lua file.
	*/
	static std::filesystem::path ContentRoot;
	static std::filesystem::path MeshesDir;
	static std::filesystem::path ScriptsDir;

};


