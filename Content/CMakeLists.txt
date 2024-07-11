
# Shaders setup
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "Shaders/*.frag"
    "Shaders/*.vert"
    "Shaders/*.comp"
)

# GLTF files
file(GLOB_RECURSE GLTF_FILES
    "GLTF/*.gltf"
    "GLTF/*.glb"
)

# Texture files
file(GLOB_RECURSE TEXTURE_FILES
    "Textures/*.png"
    "Textures/*.jpg"
)

set(SOURCE_LIST ${GLSL_SOURCE_FILES} ${GLTF_FILES} ${TEXTURE_FILES})
add_custom_target(Content SOURCES ${SOURCE_LIST})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_LIST})
