#include "AssetLoader.h"
#include <iostream>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include "Utils/Defer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

std::filesystem::path CAssetLoader::ContentRoot = "C:\\Users\\51956\\Documents\\OpenGLProjects\\GLEngine\\Content";

namespace
{
    template<typename T> 
    inline bool HasGltfError(fastgltf::Expected<T> const& value, std::filesystem::path const& filePath, const bool bAbortOnFail = false)
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

    inline GLenum ConvertGltfFilter(fastgltf::Filter filter)
    {
        const GLenum value = static_cast<GLenum>(filter);
#ifndef NDEBUG // assert that these can be cast directly to the GLenum
        switch (filter)
        {
			// nearest samplers
			case fastgltf::Filter::Nearest: assert(value == GL_NEAREST); break;
			case fastgltf::Filter::NearestMipMapNearest: assert(value == GL_NEAREST_MIPMAP_NEAREST); break;
			case fastgltf::Filter::NearestMipMapLinear: assert(value == GL_NEAREST_MIPMAP_LINEAR); break;
			// linear samplers
			case fastgltf::Filter::Linear: assert(value == GL_LINEAR); break;
			case fastgltf::Filter::LinearMipMapNearest: assert(value == GL_LINEAR_MIPMAP_NEAREST); break;
			case fastgltf::Filter::LinearMipMapLinear: assert(value == GL_LINEAR_MIPMAP_LINEAR); break;
        }
#endif
        return value;
    }
}

CAssetLoader::~CAssetLoader()
{
    // TODO: Free all resources.
}

void CAssetLoader::Create() 
{ 
    if (!AssetLoader)
    {
        AssetLoader = new CAssetLoader(); 
        AssetLoader->LoadDefaultAssets();
    }
}

void CAssetLoader::Destroy()
{
    if (AssetLoader)
    {
        delete AssetLoader;
        AssetLoader = nullptr;
    }
}

void CAssetLoader::LoadDefaultAssets()
{
	// White texture
	{
		uint32_t white = glm::packUnorm4x8(glm::vec4(1));
		glCreateTextures(GL_TEXTURE_2D, 1, &*WhiteTexture);
		glTextureStorage2D(*WhiteTexture, 1, GL_RGBA8, 1, 1);
		glTextureSubImage2D(*WhiteTexture, 0, 0, 0, 1, 1, GL_RGBA, GL_FLOAT, &white);
	}
	// Error checkerboard
	{
        constexpr size_t num64 = (16 * 16 * sizeof(uint32_t)) / sizeof(uint64_t);
		std::array<uint64_t, num64> pixels;
		uint64_t gray = glm::packUnorm4x8(glm::vec4(0.3, 0.3, 0.3, 1));
		uint64_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
        uint64_t magGray = (gray << uint64_t(32)) | magenta;
        uint64_t grayMag = (magenta << uint64_t(32)) | gray;
        for (auto it = pixels.begin(); it != pixels.end(); it += 8)
        {
            std::fill(it, it+8, magGray);
            std::swap(magGray, grayMag);
        }
        ErrorTexture = RegisterTexture2D(pixels.data(), 16, 16, 4);
		// glCreateTextures(GL_TEXTURE_2D, 1, &*ErrorTexture);
		// glTextureStorage2D(*ErrorTexture, 1, GL_RGBA8, 16, 16);
		// glTextureSubImage2D(*ErrorTexture, 0, 0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
	}
	// TODO: XYZ debug axis mesh
	{
		SMeshAsset axisMesh {};
		SVertex vertices[12] = {};
		glm::vec4 colors[] = { glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.f, 1.f) };
		for (int i = 0; i < 12; i += 2)
		{
			int axis = (i >> 1);
			vertices[i + 1].Position[axis % 3] = (i < 6) ? 1.f : -1.f;
			vertices[i].Color = vertices[i + 1].Color = (i < 6) ? colors[axis] : glm::vec4(1.f);
		}
		glCreateBuffers(1, &*axisMesh.MeshBuffers.VertexBuffer);
		glNamedBufferStorage(axisMesh.MeshBuffers.VertexBuffer, sizeof(vertices), vertices, GL_DYNAMIC_STORAGE_BIT);
        SGeoSurface surface {
            .Count = 12,
            .Material = STexturedMaterial {
                .bIgnoreLighting = true,
                .PrimitiveType = GL_LINES,
                .Diffuse = { .Texture = WhiteTexture },
                .Specular = { .Texture = WhiteTexture },
                .Shininess = 32.f
            }
        };
		axisMesh.Surfaces.push_back(surface);
        AxisMesh = std::make_shared<SMeshNode>();
        AxisMesh->LocalTransform = glm::scale(glm::mat4(1.f), glm::vec3(100.f));
        AxisMesh->RefreshTransform(glm::mat4(1.f));
        AxisMesh->Mesh = std::make_shared_for_overwrite<SMeshAsset>();
        *(AxisMesh->Mesh) = std::move(axisMesh);
	}
}

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
    Expected<fastgltf::Asset> gltf = parser.loadGltf(data.get(), filePath.parent_path(), gltfOptions);
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
                    indices.push_back(index + uint32_t(startVertex));
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
		constexpr bool bNormalsAsColors = false;
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

std::shared_ptr<SLoadedGLTF> CAssetLoader::LoadGLTFScene(std::filesystem::path filePath)
{
    // First check if we've cached this before
    filePath.make_preferred();
    const std::string filePathStr = filePath.string();
    if (auto it = SceneCache.find(filePathStr); it != SceneCache.end())
        return it->second;

    using fastgltf::Expected;
    using fastgltf::GltfDataBuffer;

    filePath = ContentRoot / filePath;
    std::cout << std::format("Loading GLTF: {}\n\tFull path = {}\n", filePathStr, filePath.string());
    Expected<GltfDataBuffer> data = GltfDataBuffer::FromPath(filePath);

    if (HasGltfError(data, filePath))
        return nullptr;

    // TODO: just use TRS instead of matrices later
    constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers; // | fastgltf::Options::DecomposeNodeMatrices;
    fastgltf::Parser parser {};
    Expected<fastgltf::Asset> gltf = parser.loadGltf(data.get(), filePath.parent_path(), gltfOptions);
    if (HasGltfError(gltf, filePath))
        return nullptr;


    std::shared_ptr<SLoadedGLTF> scene_ptr = std::make_shared<SLoadedGLTF>();
    auto& scene = *scene_ptr.get();

    // Samplers
    {
		for (fastgltf::Sampler& sampler : gltf->samplers)
		{
			SGlSamplerId id;
			glCreateSamplers(1, &*id);
			glTextureParameteri(*id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTextureParameteri(*id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			GLenum minFilter = ConvertGltfFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));
			GLenum magFilter = ConvertGltfFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));
			glTextureParameteri(*id, GL_TEXTURE_MIN_FILTER, minFilter);
			glTextureParameteri(*id, GL_TEXTURE_MAG_FILTER, magFilter); 
			scene.Samplers.emplace_back(id);
		}
    }

    // TODO: proper texture loading
    // std::vector<SGlTextureId> textures;
    // {
	// 	for (fastgltf::Image& image : gltf->images)
	// 	{
	// 		textures.emplace_back(ErrorTexture);
	// 	}
    // }


	// TODO: GLTF materials
    //std::vector<std::shared_ptr<GLTFMaterial>> materials;
    // std::vector<STexturedMaterial> materials;
    // materials.resize(gltf->materials.size(), STexturedMaterial { { ErrorTexture, {}},  { ErrorTexture, {}}, 32.f });
    // for (fastgltf::Material& mat : gltf->materials);


    // Load meshes
    std::vector<std::shared_ptr<SMeshAsset>> meshes;
    {
        std::vector<uint32_t> indices;
        std::vector<SVertex> vertices;
        for (fastgltf::Mesh& mesh : gltf->meshes)
        {
            indices.clear();
            vertices.clear();

            std::shared_ptr<SMeshAsset> newMesh_ptr = std::make_shared<SMeshAsset>();
            SMeshAsset& newMesh = *newMesh_ptr.get();
            newMesh.Name = mesh.name;
            scene.Meshes[newMesh.Name] = newMesh_ptr;


            for (auto& primitive : mesh.primitives)
            {
                SGeoSurface surface;
                surface.StartIndex = (uint32_t)indices.size();
                surface.Count = (uint32_t)gltf->accessors[primitive.indicesAccessor.value()].count;
                size_t startVertex = vertices.size();
                // Load indices
                {
                    fastgltf::Accessor& indexAccessor = gltf->accessors[primitive.indicesAccessor.value()];
                    fastgltf::iterateAccessor<uint32_t>(gltf.get(), indexAccessor, [&](uint32_t index)
                    {
                        indices.push_back(index + uint32_t(startVertex));
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
                // TODO: LoadMaterials
                // if (primitive.materialIndex)
                surface.Material = { .Diffuse = { .Texture = ErrorTexture },  .Specular = { .Texture = ErrorTexture } };
                newMesh.Surfaces.push_back(surface);
            }
            constexpr bool bNormalsAsColors = false;
            if constexpr (bNormalsAsColors)
            {
                for (auto& vtx : vertices)
                    vtx.Color = glm::vec4(vtx.Normal, 1.f);
            }

            // Create and submit glbuffers
            {
                GLuint buffers[2] = { 0, 0 }; // vbo, ibo
                glCreateBuffers(1 + !indices.empty(), buffers);
                glNamedBufferStorage(buffers[0], vertices.size() * sizeof(SVertex), vertices.data(), GL_DYNAMIC_STORAGE_BIT);

                if (!indices.empty())
                    glNamedBufferStorage(buffers[1], indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_STORAGE_BIT);

                newMesh.MeshBuffers.VertexBuffer = buffers[0];
                newMesh.MeshBuffers.IndexBuffer = buffers[1];
            }

            meshes.emplace_back(newMesh_ptr);
        }
    }

    // Load Nodes
    {
		std::vector<std::shared_ptr<SNode>> nodes;
		for (fastgltf::Node& node : gltf->nodes)
		{
			auto& newNode = nodes.emplace_back(nullptr);
			if (node.meshIndex)
			{
				auto meshNode = std::make_shared<SMeshNode>();
				newNode = meshNode;
				meshNode->Mesh = meshes[*node.meshIndex];
			}
			else
			{
				newNode = std::make_shared<SNode>();
			}
			scene.Nodes[node.name.c_str()] = newNode;

			std::visit(
				fastgltf::visitor 
				{ 
					[&](fastgltf::math::fmat4x4 matrix) 
					{ 
						memcpy(&newNode->LocalTransform, matrix.data(), sizeof(matrix));
					}, 
					[&](fastgltf::TRS transform) 
					{
						glm::vec3 tl(transform.translation[0], transform.translation[1], transform.translation[2]);
						glm::quat rot(transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2]);
						glm::vec3 sc(transform.scale[0], transform.scale[1], transform.scale[2]);
						glm::mat4 tm = glm::translate(glm::mat4(1.f), tl);
						glm::mat4 rm = glm::mat4_cast(rot);
						glm::mat4 sm = glm::scale(glm::mat4(1.f), sc);
						newNode->LocalTransform = tm * rm * sm;
					} 
				},
				node.transform);
		}
        // Set children and parents
		for (int i = 0; i < gltf->nodes.size(); i++)
		{
			fastgltf::Node& node = gltf->nodes[i];
			std::shared_ptr<SNode>& sceneNode = nodes[i];
			for (size_t& child : node.children)
			{
				sceneNode->Children.emplace_back(nodes[child]);
                nodes[child]->Parent = sceneNode;
			}
		}
        // Get root nodes
        for (auto& node : nodes)
        {
            if (node->Parent.lock() == nullptr)
            {
                scene.RootNodes.emplace_back(node);
                node->RefreshTransform(glm::mat4(1.f));
            }
        }
    }
    SceneCache[filePathStr] = scene_ptr;
    std::cout << std::format("Loaded GLTF successfully: {}\n", filePathStr);
    return scene_ptr;
}

std::optional<SGPUTexture> CAssetLoader::LoadTexture2DFromFile(std::filesystem::path const& texturePath)
{
	stbi_set_flip_vertically_on_load(true);
    SGPUTexture gpuTex {};
	std::filesystem::path p = CAssetLoader::ContentRoot / texturePath;
	p.make_preferred();
    int w, h, c;
	if (stbi_uc* texData = stbi_load(p.string().c_str(), &w, &h, &c, 0))
	{
        gpuTex.Texture = RegisterTexture2D(texData, w, h, c);
		stbi_image_free(texData);
        return gpuTex;
	}
    return std::nullopt;
}

std::optional<SGPUTexture> CAssetLoader::LoadTexture2DFromBuffer(void* buffer, int size)
{
	stbi_set_flip_vertically_on_load(true);
    SGPUTexture gpuTex {};
    int w, h, c;
	if (stbi_uc* texData = stbi_load_from_memory((stbi_uc*)buffer, size, &w, &h, &c, 0))
	{
        gpuTex.Texture = RegisterTexture2D(texData, w, h, c);
		stbi_image_free(texData);
        return gpuTex;
	}
    return std::nullopt;
}

std::optional<CGlShader> CAssetLoader::LoadShaderProgram(const std::filesystem::path& vsPath, const std::filesystem::path& fsPath)
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
    return CGlShader(program);
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

SGlTextureId CAssetLoader::RegisterTexture2D(void* stbiTexData, int width, int height, int channels)
{
	// Set sensible defaults and generate mipmaps
    SGlTextureId Id;
    assert(channels > 0 && channels != 2 && channels < 5);
    static constexpr GLenum inputFormatsByChannels[5] = { GL_NONE, GL_RED, GL_NONE, GL_RGB, GL_RGBA };
    static constexpr GLenum storageFormatsByChannels[5] = { GL_NONE, GL_R8, GL_NONE, GL_RGB8, GL_RGBA8 };
    const GLenum inputFormat = inputFormatsByChannels[channels];
    const GLenum storageFormat = storageFormatsByChannels[channels];
    glCreateTextures(GL_TEXTURE_2D, 1, &*Id);
	const int numLevels = 1 + (int)std::floor(std::log2(std::max(width, height))); 
	glTextureParameteri(Id, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTextureParameteri(Id, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	// float texBorderColor[] = { 0.0f, 1.1f, 0.08f, 1.0f };
	// glTextureParameterfv(gpuTex.Texture, GL_TEXTURE_BORDER_COLOR, texBorderColor);
	glTextureParameteri(Id, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTextureParameteri(Id, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTextureStorage2D(Id, numLevels, storageFormat, width, height);
	glTextureSubImage2D(Id, 0, 0, 0, width, height, inputFormat, GL_UNSIGNED_BYTE, stbiTexData);
	glGenerateTextureMipmap(Id);
    return Id;
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
