cmake_minimum_required (VERSION 3.8)
project("GLEngine")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GLEngine)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(ThirdParty)
add_subdirectory(Src)
add_subdirectory(Content)

add_custom_target(ReadmeEtc
	SOURCES
		README.md
		Notes.md
)
