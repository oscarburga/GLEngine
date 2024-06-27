#version 460 core

in vec3 fsNormal;
in vec3 fsFragPos;
in vec2 fsTexCoords;
in vec4 fsColor;

out vec4 FragColor;

// Material colors per phong lightning component.
struct SMaterial {
	// vec3 ambient;
	// vec3 diffuse;
	// vec3 specular;

	// diffuse map texture. Will also use this for ambient color, but could choose to use a separate ambient texture/vector
	sampler2D diffuseMap; 
	sampler2D specularMap;
	float shininess;
};

uniform vec3 viewPos;

void main()
{
	FragColor = abs(fsColor);
}
