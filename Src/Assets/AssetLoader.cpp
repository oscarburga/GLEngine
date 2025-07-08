#include "AssetLoader.h"
#include <iostream>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include "Render/GlRenderer.h"
#include "Utils/Defer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <Utils/ForEachIndexed.h>
#include <Utils/RefIgnore.h>
#undef STB_IMAGE_IMPLEMENTATION

std::filesystem::path CAssetLoader::ContentRoot = "C:\\Users\\51956\\Documents\\OpenGLProjects\\GLEngine\\Content";

namespace
{ 
	template<typename T>
	inline bool HasGltfError(fastgltf::Expected<T> const& value, std::filesystem::path const& filePath, const bool bAbortOnFail = false)
	{
		if (value.error() != fastgltf::Error::None)
		{
			std::cout << std::format("ERROR LOADING GLTF {}:\n\t{}: {}\n", filePath.string(), fastgltf::getErrorName(value.error()), fastgltf::getErrorMessage(value.error()));
			if (bAbortOnFail)
			{
				std::abort();
			}
			return true;
		}
		return false;
	}

	inline GLenum ConvertGltfWrap(fastgltf::Wrap wrap)
	{
		const GLenum value = static_cast<GLenum>(wrap);
#ifndef NDEBUG
		switch (wrap)
		{
			case fastgltf::Wrap::ClampToEdge: assert(value == GL_CLAMP_TO_EDGE); break;
			case fastgltf::Wrap::MirroredRepeat: assert(value == GL_MIRRORED_REPEAT); break;
			case fastgltf::Wrap::Repeat: assert(value == GL_REPEAT); break;
		}
#endif
		return value;
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
	if (ErrorMaterial)
		glDeleteBuffers(1, &*ErrorMaterial->DataBuffer);
	if (WhiteMaterial)
		glDeleteBuffers(1, &*WhiteMaterial->DataBuffer);
	if (AxisMesh)
	{
		glDeleteBuffers(1, &*AxisMesh->Mesh->Surfaces[0].Material->DataBuffer);
	}
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
		{
			uint32_t white = glm::packUnorm4x8(glm::vec4(1));
			WhiteTexture = RegisterTexture2D(&white, 1, 1, 4);
		}
		WhiteMaterial = std::make_shared<SPbrMaterial>();
		WhiteMaterial->Name = "DefaultWhite";
		WhiteMaterial->UboData = SPbrMaterialUboData { .bColorBound = true };
		WhiteMaterial->ColorTex.Texture = WhiteTexture;
		uint32_t ubo;
		glCreateBuffers(1, &ubo);
		glNamedBufferStorage(ubo, sizeof(SPbrMaterialUboData), &WhiteMaterial->UboData, 0);
		WhiteMaterial->DataBuffer = ubo;
	}
	// Error checkerboard
	{
		{
			constexpr size_t num64 = (16 * 16 * sizeof(uint32_t)) / sizeof(uint64_t);
			std::array<uint64_t, num64> pixels;
			uint64_t gray = glm::packUnorm4x8(glm::vec4(0.3, 0.3, 0.3, 1));
			uint64_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
			uint64_t magGray = (gray << uint64_t(32)) | magenta;
			uint64_t grayMag = (magenta << uint64_t(32)) | gray;
			for (auto it = pixels.begin(); it != pixels.end(); it += 8)
			{
				std::fill(it, it + 8, magGray);
				std::swap(magGray, grayMag);
			}
			ErrorTexture = RegisterTexture2D(pixels.data(), 16, 16, 4);
		}
		// glCreateTextures(GL_TEXTURE_2D, 1, &*ErrorTexture);
		// glTextureStorage2D(*ErrorTexture, 1, GL_RGBA8, 16, 16);
		// glTextureSubImage2D(*ErrorTexture, 0, 0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
		ErrorMaterial = std::make_shared<SPbrMaterial>();
		ErrorMaterial->Name = "DefaultChecker";
		ErrorMaterial->UboData = SPbrMaterialUboData { .bColorBound = true };
		ErrorMaterial->ColorTex.Texture = ErrorTexture;
		uint32_t ubo;
		glCreateBuffers(1, &ubo);
		glNamedBufferStorage(ubo, sizeof(SPbrMaterialUboData), &ErrorMaterial->UboData, 0);
		ErrorMaterial->DataBuffer = ubo;
	}
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
		CGlRenderer::Get()->MainVertexBuffer.Append(12, vertices);
		auto axisMaterial = std::make_shared<SPbrMaterial>(*WhiteMaterial); // Copy of the white material, but ignore lighting
		axisMaterial->bIgnoreLighting = true;
		axisMaterial->PrimitiveType = GL_LINES;
		{
			uint32_t ubo;
			glCreateBuffers(1, &ubo);
			glNamedBufferStorage(ubo, sizeof(SPbrMaterialUboData), &axisMaterial->UboData, 0);
			axisMaterial->DataBuffer = ubo;
		}
		SGeoSurface surface { 
			.Count = 12,
			.Bounds = {
				.Radius = 1,
				.Extent = { 1.f, 1.f, 1.f }
			},
			.Material = axisMaterial
		};
		axisMesh.Surfaces.push_back(surface);
		AxisMesh = std::make_shared<SMeshNode>();
		AxisMesh->LocalTransform.SetScale(glm::vec3{ 100.f });
		AxisMesh->RefreshTransform(STransform {});
		AxisMesh->Mesh = std::make_shared_for_overwrite<SMeshAsset>();
		*(AxisMesh->Mesh) = std::move(axisMesh);
	}
}

std::shared_ptr<SLoadedGLTF> CAssetLoader::LoadGLTFScene(const std::filesystem::path& gltfPath)
{
	// First check if we've cached this before
	std::filesystem::path filePath = gltfPath.is_absolute() ? gltfPath : ContentRoot / gltfPath;
	filePath = filePath.make_preferred().lexically_normal();
	const std::string filePathStr = filePath.string();
	if (auto it = SceneCache.find(filePathStr); it != SceneCache.end())
	{
		std::cout << std::format("Found GLTF {} in cache - skipping load\n", gltfPath.string());
		return it->second;
	}

	using fastgltf::Expected;
	using fastgltf::GltfDataBuffer;

	filePath = ContentRoot / filePath;
	std::cout << std::format("Loading GLTF: {}\n\tFull path = {}\n", gltfPath.string(), filePathStr);
	Expected<GltfDataBuffer> data = GltfDataBuffer::FromPath(filePath);

	if (HasGltfError(data, filePath))
		return nullptr;

	// Don't use fastgltf's DecomposeNodeMatrices. Use GLM's gtx_matrix_decompose instead. fastgltf's decomposition doesn't handle negative scales.
	constexpr auto gltfOptions = fastgltf::Options::LoadExternalBuffers; // | fastgltf::Options::DecomposeNodeMatrices;
	fastgltf::Parser parser {};
	Expected<fastgltf::Asset> gltf = parser.loadGltf(data.get(), filePath.parent_path(), gltfOptions);
	if (HasGltfError(gltf, filePath))
		return nullptr;


	std::shared_ptr<SLoadedGLTF> scene_ptr = std::make_shared<SLoadedGLTF>();
	auto& scene = *scene_ptr.get();

	// Create nodes but don't initialize their data yet
	std::vector<std::shared_ptr<SNode>> nodes;
	{
		uint32_t nodeId = 0;
		nodes.reserve(gltf->nodes.size());
		for (fastgltf::Node& node : gltf->nodes)
		{
			nodes.emplace_back(node.meshIndex ? 
				std::make_shared<SMeshNode>() : 
				std::make_shared<SNode>());
			nodes.back()->NodeId = nodeId++;
		}
	}

	// Samplers
	{
		for (fastgltf::Sampler& sampler : gltf->samplers)
		{
			SGlSamplerId id;
			glCreateSamplers(1, &*id);
			const GLenum wrapS = ConvertGltfWrap(sampler.wrapS);
			const GLenum wrapT = ConvertGltfWrap(sampler.wrapT);
			glTextureParameteri(*id, GL_TEXTURE_WRAP_S, wrapS);
			glTextureParameteri(*id, GL_TEXTURE_WRAP_T, wrapT);
			const GLenum minFilter = ConvertGltfFilter(sampler.minFilter.value_or(fastgltf::Filter::Nearest));
			const GLenum magFilter = ConvertGltfFilter(sampler.magFilter.value_or(fastgltf::Filter::Nearest));
			glTextureParameteri(*id, GL_TEXTURE_MIN_FILTER, minFilter);
			glTextureParameteri(*id, GL_TEXTURE_MAG_FILTER, magFilter);
			scene.Samplers.emplace_back(id);
		}
	}

	// Textures
	{
		for (fastgltf::Image& image : gltf->images)
		{
			std::string errMsg {};
			auto FailLoad = [&](auto&... args)
			{
				std::cerr << std::format("\tError processing image \"{}\"\n\t{}", image.name, errMsg);
				scene.Textures.emplace_back(ErrorTexture);
			};
			auto LoadFromURI = [&](fastgltf::sources::URI& imagePath)
			{
				assert(imagePath.fileByteOffset == 0);
				assert(imagePath.uri.isLocalPath());
				std::filesystem::path fullImagePath = filePath;
				fullImagePath.remove_filename();
				fullImagePath /= imagePath.uri.c_str();
				// std::cout << std::format("\tLoading image with path {}\n", imagePath.uri.c_str());
				// std::cout << std::format("\tFull path = {}\n", fullImagePath.string());
				if (auto texture = LoadTexture2DFromFile(fullImagePath, false))
				{
					// std::cout << std::format("\t\tsuccessfully loaded {}\n", imagePath.uri.c_str());
					scene.Textures.emplace_back(*texture);
				}
				else
				{
					errMsg = std::format("\tFailed to load URI image {}\n", imagePath.uri.c_str());
					FailLoad();
				}
			};
			size_t byteOffset = 0; // a bit hacky, i don't like it, but it will get the job done
			auto LoadFromBytesAndSize = [&](void* bytes, size_t size)
			{
				void* buffer = reinterpret_cast<uint8_t*>(bytes) + byteOffset;
				if (auto texture = LoadTexture2DFromBuffer(buffer, size, false))
				{
					scene.Textures.emplace_back(*texture);
				}
				else
				{
					errMsg = std::format("\tFailed to load image {} from ptr and size\n", image.name);
					FailLoad();
				}
			};
			auto LoadFromVector = [&](fastgltf::sources::Vector& vector) { LoadFromBytesAndSize(vector.bytes.data(), vector.bytes.size()); };
			auto LoadFromArray = [&](fastgltf::sources::Array& vector) { LoadFromBytesAndSize(vector.bytes.data(), vector.bytes.size()); };
			auto LoadFromBufferView = [&](fastgltf::sources::BufferView& view)
			{
				auto& bufferView = gltf->bufferViews[view.bufferViewIndex];
				auto& buffer = gltf->buffers[bufferView.bufferIndex];
				byteOffset = bufferView.byteOffset;
				std::visit(fastgltf::visitor { FailLoad, LoadFromVector, LoadFromArray }, buffer.data);
			};

			std::visit(fastgltf::visitor { FailLoad, LoadFromURI, LoadFromVector, LoadFromArray, LoadFromBufferView }, image.data);
		}
	}

	// Material
	std::vector<std::shared_ptr<SPbrMaterial>> materials;
	for (fastgltf::Material& gltfMat : gltf->materials)
	{
		std::shared_ptr<SPbrMaterial> PbrMaterial = std::make_shared<SPbrMaterial>();
		SPbrMaterial& outMat = *PbrMaterial;
		outMat.Name = gltfMat.name;
		scene.Materials[outMat.Name] = PbrMaterial;
		materials.emplace_back(PbrMaterial);

		outMat.UboData.ColorFactor.x = gltfMat.pbrData.baseColorFactor[0];
		outMat.UboData.ColorFactor.y = gltfMat.pbrData.baseColorFactor[1];
		outMat.UboData.ColorFactor.z = gltfMat.pbrData.baseColorFactor[2];
		outMat.UboData.ColorFactor.w = gltfMat.pbrData.baseColorFactor[3];

		outMat.UboData.MetalFactor = gltfMat.pbrData.metallicFactor;
		outMat.UboData.RoughFactor = gltfMat.pbrData.roughnessFactor;

		switch (gltfMat.alphaMode)
		{
			case fastgltf::AlphaMode::Opaque:
			{
				outMat.UboData.AlphaCutoff = 1.f; 
				outMat.MaterialPass = EMaterialPass::MainColor; 
				break;
			}
			case fastgltf::AlphaMode::Mask:
			{
				outMat.UboData.AlphaCutoff = gltfMat.alphaCutoff;
				outMat.MaterialPass = EMaterialPass::MainColorMasked; 
				break;
			}
			case fastgltf::AlphaMode::Blend:
			{
				outMat.UboData.AlphaCutoff = 0.01f;
				outMat.MaterialPass = EMaterialPass::Transparent; 
				break;
			}
		}

		// Default textures
		outMat.ColorTex.Texture = ErrorTexture;
		outMat.MetalRoughTex.Texture = WhiteTexture;

		// Textures
		auto SetTexture = [&](auto& gltfTexture, SGlTexture& outTex, const char* texType = "") -> bool
		{
			if (gltfTexture)
			{
				size_t texIndex = gltf->textures[gltfTexture->textureIndex].imageIndex.value();
				size_t samplerIndex = gltf->textures[gltfTexture->textureIndex].samplerIndex.value();

				outTex.Texture = scene.Textures[texIndex];
				outTex.Sampler = scene.Samplers[samplerIndex];
				return true;
			}
			const std::string alphaMode = gltfMat.alphaMode == fastgltf::AlphaMode::Opaque ? "Opaque" : 
				(gltfMat.alphaMode == fastgltf::AlphaMode::Mask ? "Masked" : "Transparent");
			std::cerr << std::format("\tMaterial {} has no {} texture (alpha mode is {})\n", gltfMat.name, texType, alphaMode);
			return false;
		};

		outMat.UboData.bColorBound = SetTexture(gltfMat.pbrData.baseColorTexture, outMat.ColorTex, "color");
		outMat.UboData.bMetalRoughBound = SetTexture(gltfMat.pbrData.metallicRoughnessTexture, outMat.MetalRoughTex, "metalRough");

		if (outMat.UboData.bNormalBound = SetTexture(gltfMat.normalTexture, outMat.NormalTex, "normal"))
			outMat.UboData.NormalScale = gltfMat.normalTexture->scale;

		if (outMat.UboData.bOcclusionBound = SetTexture(gltfMat.occlusionTexture, outMat.OcclusionTex, "occlusion"))
			outMat.UboData.OcclusionStrength = gltfMat.occlusionTexture->strength;
		// TODO emissive

		// Uniform buffer setup
		glCreateBuffers(1, &*outMat.DataBuffer);
		glNamedBufferStorage(*outMat.DataBuffer, sizeof(SPbrMaterialUboData), &outMat.UboData, 0);
	};

	// Load meshes
	std::vector<std::shared_ptr<SMeshAsset>> meshes;
	{
		std::vector<uint32_t> indices;
		std::vector<SVertex> vertices;
		std::vector<SVertexSkinData> boneData;
		bool bMultipleTexCoords = false;
		for (fastgltf::Mesh& mesh : gltf->meshes)
		{
			indices.clear();
			vertices.clear();
			boneData.clear();

			std::shared_ptr<SMeshAsset> newMesh_ptr = std::make_shared<SMeshAsset>();
			SMeshAsset& newMesh = *newMesh_ptr.get();
			newMesh.Name = mesh.name;
			scene.Meshes[newMesh.Name] = newMesh_ptr;

			bool bHasSkin = false;
			for (auto& primitive : mesh.primitives)
			{
				if (!bMultipleTexCoords && primitive.findAttribute("TEXCOORD_1") != primitive.attributes.end())
				{
					bMultipleTexCoords = true;
					std::cerr << std::format("\tError: some primitive(s) use more than one texcoord set (only 1 supported).\n");
				}

				SGeoSurface surface;
				surface.StartIndex = (uint32_t)indices.size();
				surface.Count = (uint32_t)gltf->accessors[primitive.indicesAccessor.value()].count;
				surface.Material = primitive.materialIndex ? materials[*primitive.materialIndex] : ErrorMaterial;

				size_t startVertex = vertices.size();
				// Load vertex positions & bounds, surface is ready
				{
					glm::vec3 minPos(std::numeric_limits<float>::max());
					glm::vec3 maxPos(std::numeric_limits<float>::min());
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
						minPos = glm::min(minPos, v);
						maxPos = glm::max(maxPos, v);
					});
					surface.Bounds.Origin = (maxPos + minPos) / 2.f;
					surface.Bounds.Extent = (maxPos - minPos) / 2.f;
					surface.Bounds.Radius = glm::length(surface.Bounds.Extent);
					newMesh.Surfaces.push_back(surface);
				}

				// Load indices
				{
					fastgltf::Accessor& indexAccessor = gltf->accessors[primitive.indicesAccessor.value()];
					fastgltf::iterateAccessor<uint32_t>(gltf.get(), indexAccessor, [&](uint32_t index)
					{
						indices.push_back(index + uint32_t(startVertex));
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

				// Load vertex tangents
				if (surface.Material->UboData.bNormalBound)
				{
					if (fastgltf::Attribute* tangents = primitive.findAttribute("TANGENT"); tangents != primitive.attributes.end())
					{
						fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf.get(), gltf->accessors[tangents->accessorIndex], [&](glm::vec4 v, size_t index)
						{
							vertices[startVertex + index].Tangent = v;
						});
					}
					else
					{
						// No tangents found and don't want to implement calculating them, just disable normals on this material.
						surface.Material->UboData.bNormalBound = false;
						std::cerr << std::format("\tOne of the primitives uses a normal-mapped material, but provides no tangents.\n"
							"\t\tFallback: setting material {} to not use normal map\n", surface.Material->Name);
					}
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

				// SKINNING
				// Try load vertex joints & weights
				fastgltf::Attribute* joints = primitive.findAttribute("JOINTS_0");
				fastgltf::Attribute* weights = primitive.findAttribute("WEIGHTS_0");
				if (joints != primitive.attributes.end() && weights != primitive.attributes.end())
				{
					bHasSkin = true;
					boneData.resize(vertices.size(), {});
					fastgltf::iterateAccessorWithIndex<glm::uvec4>(gltf.get(), gltf->accessors[joints->accessorIndex], [&](glm::uvec4 value, size_t index)
					{
						boneData[startVertex + index].Joints = value;
					});
					fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf.get(), gltf->accessors[weights->accessorIndex], [&](glm::vec4 value, size_t index)
					{
						boneData[startVertex + index].Weights = value;
					});
				}

			}

			// Ensure boneData size ends up matching vertex size in case the last primitive(s) didn't have skin data
			if (bHasSkin)
				boneData.resize(vertices.size(), {});

			// Create and submit vertex buffers
			{
				// BufferVector append already returns empty ID if passing in an empty vector - no need to manually check.
				newMesh.VertexBuffer = CGlRenderer::Get()->MainVertexBuffer.Append(vertices);
				newMesh.IndexBuffer = CGlRenderer::Get()->MainIndexBuffer.Append(indices);
				newMesh.VertexJointsDataBuffer = CGlRenderer::Get()->MainBonesBuffer.Append(boneData);
			}
			meshes.emplace_back(newMesh_ptr);
		}
	}

	// Load skins
	std::vector<std::shared_ptr<SSkinAsset>> skins;
	std::vector<std::vector<uint32_t>> skinsNodeIsJointOf(nodes.size(), std::vector<uint32_t>{}); // Really annoying 1 node can be joint of multiple skins... unrealistic case but allowed
	{
		for (fastgltf::Skin& gltfSkin : gltf->skins)
		{
			SSkinAsset& skin = *skins.emplace_back(std::make_shared<SSkinAsset>());
			skin.Name = gltfSkin.name;
			scene.Skins[skin.Name] = skins.back();

			if (gltfSkin.skeleton)
				skin.SkeletonRoot = nodes[*gltfSkin.skeleton];

			skin.AllJoints.resize(gltfSkin.joints.size(), SJoint {});
			util::for_each_indexed(gltfSkin.joints.begin(), gltfSkin.joints.end(), 0, [&](uint32_t i, size_t joint)
			{
				assert(joint >= 0);
				skin.AllJoints[i].JointId = i;
				skin.AllJoints[i].Node = nodes[joint];
				skinsNodeIsJointOf[joint].push_back(uint32_t(skins.size()) - 1);
			});
			if (gltfSkin.inverseBindMatrices)
			{
				auto& ibmAccessor = gltf->accessors[*gltfSkin.inverseBindMatrices];
				assert(ibmAccessor.type == fastgltf::AccessorType::Mat4);
				fastgltf::iterateAccessorWithIndex<glm::mat4>(gltf.get(), ibmAccessor, [&](glm::mat4 const& mat, size_t index)
				{
					skin.AllJoints[index].InverseBindMatrix = mat;
				});
			}
			std::cout << std::format("\tLoaded skin {}, has IB matrices = {}\n", skin.Name, gltfSkin.inverseBindMatrices.has_value());
		}
	}

	// Load animations
	// Want to load the asset as single skeletal meshes with animations per skeletal mesh (like a game engine typically would),
	// so will parse the animation per skin (a single gltf animation can animate any/all nodes in a scene, we want to separate it into 
	// animations per skin, and discard non-joint animations).
	{
		std::vector<SAnimationAsset> animsPerSkin(skins.size());
		for (fastgltf::Animation& gltfAnim : gltf->animations)
		{
			std::cout << std::format("\tLoading animation {}\n", gltfAnim.name);
			for (SAnimationAsset& anim : animsPerSkin)
			{
				anim.AnimationLength = 0.0f;
				anim.JointKeyFrames.clear();
				anim.OwnerSkin.reset();
			}
			
			// Remove anims without node
			std::erase_if(gltfAnim.channels, [&](auto& channel) { return !channel.nodeIndex; });

			// Sort channels by node id
			std::sort(gltfAnim.channels.begin(), gltfAnim.channels.end(), [&](auto& c1, auto& c2)
			{
				return *c1.nodeIndex < *c2.nodeIndex;
			});

			// channels are now guaranteed to always have a valid node and be sorted by increasing node id
			for (fastgltf::AnimationChannel& channel : gltfAnim.channels)
			{
				SNode* targetNode = nodes[*channel.nodeIndex].get();
				// TODO: MORPH TARGETS
				if (channel.path == fastgltf::AnimationPath::Weights)
				{
					std::cout << std::format("\t\tAnimation {}: skipping channel that animates weights (morph targets) for node {} ({}). Currently not supported\n",
						gltfAnim.name, *channel.nodeIndex, gltf->nodes[*channel.nodeIndex].name);
					continue;
				}
				// TODO: CUBIC SPLINE INTERPOLATION
				auto& sampler = gltfAnim.samplers[channel.samplerIndex];
				if (sampler.interpolation == fastgltf::AnimationInterpolation::CubicSpline)
				{
					std::cout << std::format("\t\tAnimation {}: skipping channel that animates interpolates using cubic spline for node {} ({})\n",
						gltfAnim.name, *channel.nodeIndex, gltf->nodes[*channel.nodeIndex].name);
					continue;
				}
				auto& samplerInput = gltf->accessors[sampler.inputAccessor];
				assert(samplerInput.type == fastgltf::AccessorType::Scalar);
				assert(samplerInput.componentType == fastgltf::ComponentType::Float || samplerInput.componentType == fastgltf::ComponentType::Double);
				auto& samplerOutput = gltf->accessors[sampler.outputAccessor];
				assert(samplerInput.count == samplerOutput.count);
				if (skinsNodeIsJointOf[*channel.nodeIndex].empty())
				{
					std::cout << std::format("\t\tAnimation {}: channel animates node {} ({}), but this node is not part of any skins.\n",
						gltfAnim.name, *channel.nodeIndex, gltf->nodes[*channel.nodeIndex].name);
				}

				for (uint32_t skinIdx : skinsNodeIsJointOf[*channel.nodeIndex])
				{
					std::vector<SJointAnimData>& skinKeyFrames = animsPerSkin[skinIdx].JointKeyFrames;

					if (skinKeyFrames.empty() || skinKeyFrames.back().JointNode != targetNode) // .back() comparison works because channels are sorted by node id
						skinKeyFrames.emplace_back(SJointAnimData { .JointNode = targetNode });

					SJointAnimData& jointAnimData = skinKeyFrames.back();
					for (size_t kfIndex = 0; kfIndex < samplerInput.count; ++kfIndex)
					{
						float timeStamp = fastgltf::getAccessorElement<float>(gltf.get(), samplerInput, kfIndex);
						animsPerSkin[skinIdx].AnimationLength = std::max(animsPerSkin[skinIdx].AnimationLength, timeStamp);
						switch (channel.path)
						{
							case fastgltf::AnimationPath::Translation:
							{
								glm::vec3 pos = fastgltf::getAccessorElement<glm::vec3>(gltf.get(), samplerOutput, kfIndex);
								jointAnimData.Positions.emplace_back(SKeyFrame<glm::vec3> { timeStamp, pos });
								break;
							}
							case fastgltf::AnimationPath::Rotation:
							{
								glm::vec4 rotVec = fastgltf::getAccessorElement<glm::vec4>(gltf.get(), samplerOutput, kfIndex);
								glm::quat rotQuat { rotVec.w, rotVec.x, rotVec.y, rotVec.z };
								jointAnimData.Rotations.emplace_back(SKeyFrame<glm::quat> { timeStamp, rotQuat });
								break;
							}
							case fastgltf::AnimationPath::Scale:
							{
								glm::vec3 scale = fastgltf::getAccessorElement<glm::vec3>(gltf.get(), samplerOutput, kfIndex);
								jointAnimData.Scales.emplace_back(SKeyFrame<glm::vec3> { timeStamp, scale });
								break;
							}
						}
					}
				}
			}

			util::for_each_indexed(animsPerSkin.begin(), animsPerSkin.end(), 0, [&](int i, SAnimationAsset& anim)
			{
				if (anim.JointKeyFrames.empty())
					return;
				anim.OwnerSkin = skins[i];
				skins[i]->Animations[gltfAnim.name.c_str()] = std::move(anim);
				std::cout << std::format("\t\tAdded animation {} to skin {}\n", gltfAnim.name, skins[i]->Name);
			});
		}
	}

	// Actually load nodes
	{
		util::for_each_indexed(gltf->nodes.begin(), gltf->nodes.end(), 0, [&](uint32_t i, fastgltf::Node& node)
		{
			auto& newNode = nodes[i];
			scene.Nodes[node.name.c_str()] = newNode;

			if (node.meshIndex)
			{
				SMeshNode* meshNode = static_cast<SMeshNode*>(newNode.get());
				meshNode->Mesh = meshes[*node.meshIndex];
				if (node.skinIndex)
				{
					meshNode->Skin = skins[*node.skinIndex];
				}
			}

			std::visit(
				fastgltf::visitor
				{
					[&](fastgltf::math::fmat4x4 matrix)
					{
						glm::mat4* mat = reinterpret_cast<glm::mat4*>(&matrix);
						newNode->LocalTransform = STransform { *mat };
					},
					[&](fastgltf::TRS transform)
					{
						glm::vec3 pos = glm::make_vec3(&transform.translation[0]);
						glm::quat rot { transform.rotation[3], transform.rotation[0], transform.rotation[1], transform.rotation[2] };
						glm::vec3 scale = glm::make_vec3(&transform.scale[0]);
						newNode->LocalTransform = STransform { pos, rot, scale };
					}
				},
				node.transform);
			newNode->OriginalLocalTransform = newNode->LocalTransform;
		});
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
				node->RefreshTransform();
			}
		}
	}

	// Now that all nodes are loaded and have their transforms, initialize the skin animators
	for (auto& skin : skins)
	{
		skin->InitAnimator();
	}

	scene.UserTransform = STransform {};
	SceneCache[filePathStr] = scene_ptr;
	std::cout << std::format("Loaded GLTF successfully: {}\n", gltfPath.string());
	return scene_ptr;
}

std::optional<SGlTextureId> CAssetLoader::LoadTexture2DFromFile(std::filesystem::path const& texturePath, bool bFlipVertical)
{
	stbi_set_flip_vertically_on_load(bFlipVertical);
	std::filesystem::path p = texturePath.is_absolute() ? texturePath : CAssetLoader::ContentRoot / texturePath;
	p.make_preferred();
	int w, h, c;
	if (stbi_uc* texData = stbi_load(p.string().c_str(), &w, &h, &c, 0))
	{
		SGlTextureId id = RegisterTexture2D(texData, w, h, c);
		stbi_image_free(texData);
		return id;
	}
	else
	{
		std::cerr << std::format("LoadTexture2DFromFile failed:\n\t{}\n", stbi_failure_reason());
	}
	return std::nullopt;
}

std::optional<SGlTextureId> CAssetLoader::LoadTexture2DFromBuffer(void* buffer, size_t size, bool bFlipVertical)
{
	stbi_set_flip_vertically_on_load(bFlipVertical);
	int w, h, c;
	if (stbi_uc* texData = stbi_load_from_memory((stbi_uc*)buffer, (int)size, &w, &h, &c, 0))
	{
		SGlTextureId id = RegisterTexture2D(texData, w, h, c);
		stbi_image_free(texData);
		return id;
	}
	else
	{
		std::cerr << std::format("LoadTexture2DFromBuffer failed:\n\t{}\n", stbi_failure_reason());
	}
	return std::nullopt;
}

std::optional<CGlShader> CAssetLoader::LoadShaderProgram(const SShaderLoadArgs& vsArgs, const SShaderLoadArgs& fsArgs)
{
	return LoadShaderProgram(vsArgs, SShaderLoadArgs {}, fsArgs);
}

std::optional<CGlShader> CAssetLoader::LoadShaderProgram(const SShaderLoadArgs& vsArgs, const SShaderLoadArgs& gsArgs, const SShaderLoadArgs& fsArgs)
{
	auto vs = LoadSingleShader(vsArgs, GL_VERTEX_SHADER);
	auto gs = LoadSingleShader(gsArgs, GL_GEOMETRY_SHADER);
	auto fs = LoadSingleShader(fsArgs, GL_FRAGMENT_SHADER);

	// Vertex and fragment shader are mandatory. 
	// Geometry shader is optional, but if a non-empty path was provided, have to check it compiled successfully
	if (!vs || !fs || (!gsArgs.Path.empty() && !gs))
		return std::nullopt;

	auto destroyShaders = Defer([&]()
	{
		if (vs)
			glDeleteShader(*vs);

		if (gs)
			glDeleteShader(*gs);

		if (fs)
			glDeleteShader(*fs);
	});

	unsigned int program = glCreateProgram();
	glAttachShader(program, *vs);
	if (gs)
		glAttachShader(program, *gs);
	glAttachShader(program, *fs);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
		std::cerr << std::format("Program failed to link:\nVertex Shader:{}\nGeometry Shader: {}\nFragment Shader: {}\nLog:\n{}\n",
			vsArgs.Path.string(), gsArgs.Path.string(), fsArgs.Path.string(), infoLog);
		glDeleteProgram(program);
		return std::nullopt;
	}
	return CGlShader(program);
}

std::optional<std::string> CAssetLoader::ReadContentFileToString(const std::filesystem::path& filePath)
{
	return ReadFileToString(ContentRoot / filePath);
}

std::optional<std::string> CAssetLoader::ReadFileToString(const std::filesystem::path& rawFilePath)
{
	// open at end, read as binary since we just want the flat raw content, don't care about any text format.
	const std::filesystem::path& path = rawFilePath;
	std::ifstream fileStream(path, std::ios_base::ate | std::ios_base::binary);
	if (!fileStream)
	{
		std::cerr << "Failed to open shader file " << path << '\n';
		return std::nullopt;
	}
	size_t codeLen = fileStream.tellg();
	std::string fileContents(codeLen + 1, 0); // +1 to include a null terminator
	fileStream.seekg(0);
	fileStream.read(fileContents.data(), codeLen);
	fileContents.back() = 0; // null-terminated string
	fileStream.close();
	return std::optional { std::move(fileContents) };
}

std::optional<unsigned int> CAssetLoader::LoadSingleShader(const SShaderLoadArgs& shaderArgs, unsigned int shaderType)
{
	if (shaderArgs.Path.empty())
		return std::nullopt;

	if (auto shaderCodeOpt = ReadContentFileToString(shaderArgs.Path); shaderCodeOpt.has_value())
	{

		GLuint shader = glCreateShader(shaderType);
		std::string replacedCode = shaderArgs.ApplyToCode(std::move(*shaderCodeOpt));
		const char* shaderStr = replacedCode.c_str();
		glShaderSource(shader, 1, &shaderStr, nullptr);
		glCompileShader(shader);
		if (!CheckShaderCompilation(shader, shaderArgs.Path))
		{
			glDeleteShader(shader);
			return std::nullopt;
		}
		return shader;
	}
	return std::nullopt;
}

SGlTextureId CAssetLoader::RegisterTexture2D(void* stbiTexData, int width, int height, int channels)
{
	// Set sensible defaults and generate mipmaps
	SGlTextureId Id;
	assert(channels > 0 && channels < 5);
	static constexpr GLenum inputFormatsByChannels[5] = { GL_NONE, GL_RED, GL_RG, GL_RGB, GL_RGBA };
	static constexpr GLenum storageFormatsByChannels[5] = { GL_NONE, GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
	const GLenum inputFormat = inputFormatsByChannels[channels];
	const GLenum storageFormat = storageFormatsByChannels[channels];
	glCreateTextures(GL_TEXTURE_2D, 1, &*Id);
	const int numLevels = 1 + (int)std::floor(std::log2(std::max(width, height)));
	glTextureParameteri(Id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(Id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

std::string SShaderLoadArgs::ApplyToCode(std::string&& shaderCode) const
{
	if (Args.empty())
		return std::string { std::move(shaderCode) };

	constexpr std::string_view argsBeginStr = "#define COMPILEARG_BEGIN";
	constexpr std::string_view argsEndStr = "#define COMPILEARG_END";

	size_t beginPos = shaderCode.find(argsBeginStr);
	if (beginPos == std::string::npos)
		return std::string { shaderCode };

	size_t firstPosAfterBegin = beginPos + argsBeginStr.size();
	size_t endPos = shaderCode.find(argsEndStr, firstPosAfterBegin);
	if (endPos == std::string::npos)
		return std::string { shaderCode };

	std::string result;
	result.reserve(shaderCode.size() * 2);
	std::copy(shaderCode.begin(), shaderCode.begin() + beginPos, std::back_inserter(result));
	std::stringstream ss(shaderCode.substr(beginPos, endPos - beginPos));
	std::getline(ss, util::RefIgnore<std::string>::I); // discard compilearg_begin line

	for (int lineNum = 1; ss; lineNum++)
	{
		constexpr std::string_view defineStr = "#define";
		std::string line, define, argName, defaultValueRaw = "";
		std::getline(ss, line);
		std::stringstream linestream(line);
		linestream >> define;
		linestream >> argName;
		// linestream >> defaultValue; // This doesn't work for all cases, need to get the entire remainder of the line (without trailing whitespaces);
		const std::string_view defaultValueView = [&]
		{
			std::getline(linestream, defaultValueRaw);
			while (defaultValueRaw.size() && std::isspace(defaultValueRaw.back())) // remove trailing whitespace
				defaultValueRaw.pop_back();

			size_t i = 0;
			while (i < defaultValueRaw.size() && std::isspace(defaultValueRaw[i]))
				++i;

			if (i < defaultValueRaw.size())
				return std::string_view(defaultValueRaw.begin() + i, defaultValueRaw.end());

			return std::string_view {};
		}();

		if (!define.starts_with(defineStr) || argName.empty())
		{
			std::cerr << std::format("\tSkipping Args line {}: '{}'\n", lineNum, line);
			continue;
		}
		result += std::format("{} {}", defineStr, argName);
		if (auto it = Args.find(argName); it != Args.end())
		{
			const std::string& newArgVal = it->second;
			std::cout << std::format("\tSetting arg '{}' to value '{}' - default was '{}'\n", argName, newArgVal, defaultValueView);
			if (newArgVal.size())
			{
				result += ' ';
				result += newArgVal;
			}
		}
		else
		{
			std::cout << std::format("\tArg {} remains unchanged. Default value is {}\n", argName, defaultValueView);
			if (defaultValueView.size())
			{
				result += ' ';
				result += defaultValueView;
			}
		}
		result += '\n';
	}
	std::copy(shaderCode.begin() + endPos, shaderCode.end(), std::back_inserter(result));
	return result;
}
