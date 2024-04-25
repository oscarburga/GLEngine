#version 460 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

// Material colors per phong lightning component.
struct Material {
	// vec3 ambient;
	// vec3 diffuse}
	// diffuse map texture. Will also use this for ambient color, but could choose to use a separate ambient texture/vector
	sampler2D diffuseMap; 
	sampler2D specularMap;
	// vec3 specular;
	float shininess;
};

// Light position + colors per phong lightning component.
struct Light {
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;
uniform vec3 objectColor;

void main()
{
	const vec3 diffuseTexColor = texture(material.diffuseMap, TexCoords).xyz;
	const vec3 specularTexColor = texture(material.specularMap, TexCoords).xyz;
	const vec3 fragNormal = normalize(Normal);
	const vec3 dirToLight = normalize(light.position - FragPos);

	float diffuseStrength = max(dot(fragNormal, dirToLight), 0.0f);
	// vec3 diffuseColor = light.diffuse * (diffuseStrength * material.diffuse);
	vec3 diffuseColor = light.diffuse * (diffuseStrength * diffuseTexColor);

	// vec3 ambientColor = material.ambient * light.ambient;
	vec3 ambientColor = diffuseTexColor * light.ambient;

	vec3 dirToCamera = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-dirToLight, fragNormal); // reflect(I, S) expects the incident vector I to point into the surface defined by the normal vector S
	float specularResult = pow(max(dot(dirToCamera, reflectDir), 0.0f), material.shininess);
	vec3 specularColor = (specularTexColor * specularResult) * light.specular;

	vec3 resultColor = (diffuseColor + ambientColor + specularColor);
	FragColor = vec4(resultColor, 1.0f);
}
