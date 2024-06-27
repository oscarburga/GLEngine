#include "AssetLoader.h"
#include "glad/glad.h"
#include <iostream>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <Utils/Defer.h>

std::filesystem::path CAssetLoader::ContentRoot = "C:\\Users\\51956\\Documents\\OpenGLProjects\\GLEngine\\Content";

namespace
{
    template<typename T> 
    inline bool HasGltfError(fastgltf::Expected<T> const& value, std::filesystem::path const& filePath, const bool bAbortOnFail = true)
    {
		if (value.error() != fastgltf::Error::None)
        {
			std::cout << std::format("ERROR LOADING GLTF {}\n: {}: {}", filePath.string(), fastgltf::getErrorName(value.error()), fastgltf::getErrorMessage(value.error()));
            if (bAbortOnFail)
            {
                std::abort();
            }
            return true;
		}
        return false;
    }
}

#define GLTF_CHECK(data, filePath)\

std::optional<std::vector<SMeshAsset>> CAssetLoader::LoadGLTFMeshes(std::filesystem::path filePath)
{
    using fastgltf::Expected;
    using fastgltf::GltfDataBuffer;

    filePath = ContentRoot / filePath;
    std::cout << std::format("Loading GLTF: {}\n", filePath.string());
    Expected<GltfDataBuffer> data = GltfDataBuffer::FromPath(filePath);

    if (HasGltfError(data, filePath))
        return std::nullopt;

    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers; // | fastgltf::Options::LoadGLBBuffers;
    fastgltf::Parser parser {};
    Expected<fastgltf::Asset> gltf = parser.loadGltfBinary(data.get(), filePath.parent_path(), gltfOptions);
    if (HasGltfError(gltf, filePath))
        return std::nullopt;

    std::vector<SMeshAsset> meshes;
    std::vector<uint32_t> indices;
    std::vector<SVertex> vertices;

    for (fastgltf::Mesh& mesh : gltf->meshes)
    {
        indices.clear();
        vertices.clear();

        SMeshAsset newMesh;
        newMesh.Name = mesh.name; // Possibly can just move the name?

        for (auto& primitive : mesh.primitives)
        {
            SGeoSurface surface;
            surface.StartIndex = (uint32_t)indices.size();
            surface.Count = (uint32_t)gltf->accessors[primitive.indicesAccessor.value()].count;
            newMesh.Surfaces.push_back(surface);

            size_t startVertex = vertices.size();
            // Load indices
            {
                fastgltf::Accessor& indexAccessor = gltf->accessors[primitive.indicesAccessor.value()];
                fastgltf::iterateAccessor<uint32_t>(gltf.get(), indexAccessor, [&](uint32_t index)
                {
                    indices.push_back(index + startVertex);
                });
            }

            // Load vertex positions
            {
				fastgltf::Accessor& posAccessor = gltf->accessors[primitive.findAttribute("POSITION")->accessorIndex];
				vertices.resize(vertices.size() + posAccessor.count);
				fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf.get(), posAccessor, [&](glm::vec3 v, size_t index)
				{
					vertices[startVertex + index] = {
						.Position = v,
						.uv_x = 0.0f,
						.Normal = { 1, 0, 0 },
						.uv_y = 0.0f,
						.Color = { 1, 1, 1, 1 }
					};
				});
            }

            // Load vertex normals
            if (fastgltf::Attribute* normals = primitive.findAttribute("NORMAL"); normals != primitive.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf.get(), gltf->accessors[normals->accessorIndex], [&](glm::vec3 v, size_t index) 
                {
                    vertices[startVertex + index].Normal = v;
                });
            }

            // Load UVs
            if (fastgltf::Attribute* uvs = primitive.findAttribute("TEXCOORD_0"); uvs != primitive.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf.get(), gltf->accessors[uvs->accessorIndex], [&](glm::vec2 v, size_t index) 
                {
                    vertices[startVertex + index].uv_x = v.x;
                    vertices[startVertex + index].uv_y = v.y;
                });
            }

            // Load vertex colors
            if (fastgltf::Attribute* colors = primitive.findAttribute("COLOR_0"); colors != primitive.attributes.end())
            {
                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf.get(), gltf->accessors[colors->accessorIndex], [&](glm::vec4 v, size_t index) 
                {
					vertices[startVertex + index].Color = v;
                });
            }
            // TODO: Load textures
        }
		constexpr bool bNormalsAsColors = true;
		if constexpr (bNormalsAsColors)
		{
            for (auto& vtx : vertices)
                vtx.Color = glm::vec4(vtx.Normal, 1.f);
		}
        GLuint buffers[2] = { 0, 0 }; // vbo, ibo
        glCreateBuffers(1 + !indices.empty(), buffers);
        glNamedBufferStorage(buffers[0], vertices.size() * sizeof(SVertex), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

        if (!indices.empty())
			glNamedBufferStorage(buffers[1], indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_STORAGE_BIT);

        newMesh.MeshBuffers.VertexBuffer = buffers[0];
        newMesh.MeshBuffers.IndexBuffer = buffers[1];

        meshes.emplace_back(std::move(newMesh));
    }
    return meshes;
}

std::optional<CShader> CAssetLoader::LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
{
    auto vs = LoadSingleShader(vsPath, GL_VERTEX_SHADER);
    auto fs = LoadSingleShader(fsPath, GL_FRAGMENT_SHADER);
    auto destroyShaders = Defer([&]()
    {
        if (vs)
            glDeleteShader(*vs);

        if (fs)
            glDeleteShader(*fs);
    });

    unsigned int program = glCreateProgram();
	glAttachShader(program, *vs);
    glAttachShader(program, *fs);
    glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << std::format("Program failed to link:\nVertex Shader:{}\nFragment Shader: {}\nLog:\n{}\n", 
            vsPath.string(), fsPath.string(), infoLog);
        glDeleteProgram(program);
        return std::nullopt;
	}
    return CShader(program);
}

std::optional<unsigned int> CAssetLoader::LoadSingleShader(const std::filesystem::path& shaderPath, unsigned int shaderType)
{
	// shaderCode.clear();
	std::stringstream shaderStream;
	// open at end, read as binary since we just want the flat raw content, don't care about any text format.
    std::filesystem::path path = ContentRoot / shaderPath;
	std::ifstream shaderFile(path, std::ios_base::ate | std::ios_base::binary); 
	if (!shaderFile)
	{
		std::cerr << "Failed to open shader file " << path << '\n';
		return std::nullopt;
	}
	size_t codeLen = shaderFile.tellg();
    std::string shaderCode(codeLen + 1, 0);
    shaderCode.resize(codeLen + 1); // +1 for null terminator
	shaderFile.seekg(0);
	shaderFile.read(shaderCode.data(), codeLen);
	shaderCode.back() = 0; // null-terminated string
	shaderFile.close();

	GLuint shader = glCreateShader(shaderType);
	const char* shaderStr = shaderCode.c_str();
	glShaderSource(shader, 1, &shaderStr, nullptr);
	glCompileShader(shader);
    if (!CheckShaderCompilation(shader, path))
    {
        glDeleteShader(shader);
        return std::nullopt;
    }
    return shader;
}

bool CAssetLoader::CheckShaderCompilation(unsigned int shader, const std::filesystem::path& shaderPath)
{
	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << std::format("Error - Shader compilation failed\nShaderPath: {}\n Error log:\n{}\n",
            shaderPath.string(),
            infoLog);
        return false;
	}
    return true;
}
