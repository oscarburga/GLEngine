set(SOURCE_LIST 
	main.cpp

	# ASSETS
	Assets/AssetLoader.h
	Assets/AssetLoader.cpp
	
	# MATH
	Math/EngineMath.h
	Math/EngineMath.cpp
	Math/Bounds.h
	Math/Frustum.h
	Math/Frustum.cpp
	Math/Plane.h
	Math/Plane.cpp
	
	# RENDERING
	Render/GeoSurface.h
	Render/GlBufferVector.h
	Render/GlBufferVector.cpp
	Render/GlCamera.h
	Render/GlCamera.cpp
	Render/GlDrawCommands.h
	Render/GlDrawCommands.cpp
	Render/GlIdTypes.h
	Render/GlRenderer.h
	Render/GlRenderer.cpp
	Render/GlShader.h
	Render/GlShader.cpp
	Render/GlShadowDepth.cpp
	Render/GlShadowDepth.h
	Render/Materials.h
	Render/Materials.cpp
	Render/RenderObject.h
	Render/RenderObject.cpp
	Render/SceneData.h
	Render/SceneGraph.h
	Render/SceneGraph.cpp
	Render/Skinning.h
	Render/Skinning.cpp
	Render/VertexStructs.h

	# UTILS
	Utils/GenericConcepts.h
	Utils/Defer.h
	Utils/ForEachIndexed.h
	Utils/FuncVector.h
	Utils/MultiTypeContainer.h
	Utils/RefIgnore.h
	Utils/IdType.h

	# TOOLS
	Tools/ImguiTools.h
	Tools/ImguiTools.cpp
	
	# GAME / UNCATEGORIZED
	Engine.h
	Engine.cpp
	InputConcepts.h
)

add_executable(GLEngine ${SOURCE_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})

set_property(TARGET GLEngine PROPERTY CXX_STANDARD 20)
target_include_directories(GLEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(GLEngine PUBLIC glm glad stb_image glfw fastgltf::fastgltf imgui)
target_precompile_headers(GLEngine PUBLIC
	<functional>
	<memory>
	<numbers>
	<numeric>
	<optional>
	<string>
	<unordered_map>
	<vector>

	<glm/glm.hpp>
	<glm/gtc/matrix_transform.hpp>
	<glm/gtc/type_ptr.hpp>
	<glm/gtc/random.hpp>
)