cmake_minimum_required (VERSION 3.8)
project ("GLEngine")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GLEngine)

add_subdirectory(ThirdParty)
add_subdirectory(Src)

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
    )

add_custom_target(Shaders 
    SOURCES ${GLSL_SOURCE_FILES}
)