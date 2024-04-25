#version 460 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

// Material colors per phong lightning component.
struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
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
	vec3 fragNormal = normalize(Normal);
	vec3 dirToLight = normalize(light.position - FragPos);

	float diffuseStrength = max(dot(fragNormal, dirToLight), 0.0f);
	vec3 diffuseColor = light.specular * (diffuseStrength * material.diffuse);

	vec3 ambientColor = material.ambient * light.ambient;

	vec3 dirToCamera = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-dirToLight, fragNormal); // reflect(I, S) expects the incident vector I to point into the surface defined by the normal vector S
	float specularResult = pow(max(dot(dirToCamera, reflectDir), 0.0f), material.shininess);
	vec3 specularColor = (material.specular * specularResult) * light.specular;

	vec3 resultColor = (diffuseColor + ambientColor + specularColor);
	FragColor = vec4(resultColor, 1.0f);
}
