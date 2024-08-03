add_library(glad STATIC)
target_sources(glad PRIVATE
	glad/glad/glad.h
	glad/glad/glad.c
	glad/KHR/khrplatform.h
)
target_include_directories(glad PUBLIC glad)


add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE stb_image)

add_subdirectory(fastgltf)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

add_library(imgui STATIC)
target_sources(imgui PRIVATE 
	imgui/imgui.h
	imgui/imgui.cpp
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_widgets.cpp
	imgui/imgui_tables.cpp
	imgui/backends/imgui_impl_opengl3.cpp
	imgui/backends/imgui_impl_glfw.cpp

	imgui/misc/debuggers/imgui.natvis
	imgui/misc/debuggers/imgui.natstepfilter
)
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PUBLIC glfw)
