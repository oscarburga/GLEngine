file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
    "shaders/*.comp"
    )

add_custom_target(Shaders 
    SOURCES ${GLSL_SOURCE_FILES}
)