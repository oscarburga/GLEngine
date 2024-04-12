#version 460 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

void main()
{
	vec3 fragNormal = normalize(Normal);
	vec3 dirToLight = normalize(lightPos - FragPos);
	float diffuseStrength = max(dot(fragNormal, dirToLight), 0.0f);
	vec3 diffuseColor = diffuseStrength * lightColor;

	float ambientStrength = 0.1f;
	vec3 ambientColor = ambientStrength * lightColor;

	float specularStrength = 0.5f;
	vec3 dirToCamera = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-dirToLight, fragNormal); // reflect(I, S) expects the incident vector I to point into the surface defined by the normal vector S
	float specularResult = pow(max(dot(dirToCamera, reflectDir), 0.0f), 32);
	vec3 specularColor = (specularStrength * specularResult) * lightColor;

	vec3 colorFromLightning = (diffuseColor + ambientColor + specularColor);
	FragColor = vec4(colorFromLightning * objectColor, 1.0f);
}
