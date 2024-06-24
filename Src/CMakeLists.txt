add_executable (GLEngine 
	main.cpp
	Camera.h
	Camera.cpp
	Engine.h
	Engine.cpp
	EngineMath.h
	EngineMath.cpp
	InputConcepts.h
	MultiTypeContainer.h
	RefIgnore.h
	Shader.h
	Shader.cpp
	World.h
	WorldContainers.h
	WorldContainers.cpp
	WorldObject.h
	WorldObject.cpp
)

set_property(TARGET GLEngine PROPERTY CXX_STANDARD 20)
target_include_directories(GLEngine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(GLEngine PUBLIC glm glad stb_image glfw fastgltf::fastgltf)
target_precompile_headers(GLEngine PUBLIC
	<optional>
	<vector>
	<memory>
	<string>
	<vector>
	<numeric>
	<numbers>
	<unordered_map>
	<glm/glm.hpp>
	<glm/gtc/matrix_transform.hpp>
	<glm/gtc/type_ptr.hpp>
	<glm/gtc/random.hpp>
)