#version 460 core

in vec3 fsNormal;
in vec3 fsFragPos;
in vec2 fsTexCoords;
in vec4 fsColor;

out vec4 FragColor;

struct SMaterial {
	vec3 ambient;
	vec3 diffuse; 
	vec3 specular;
	float shininess;
};

/* Light position + colors per phong lightning component. Includes data for all 
*  directional light, point light and spot lights to avoid duplicating code.  */
struct SLight {
	vec3 direction;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float constant; // Should be 1 basically always.
	float linear; // Set to zero for directional light
	float quadratic; // Set to zero for directional light
	float innerCutoff;
	float outerCutoff;
};

#define MAX_POINT_LIGHTS 4

uniform SMaterial material;
uniform SLight pointLights[MAX_POINT_LIGHTS];
uniform SLight spotLight;
uniform vec3 viewPos;
uniform vec3 objectColor;


vec3 calcLight(SLight light) 
{
	if (light.constant == 0.0f) 
		return vec3(0.0f);
	const vec3 diffuseTexColor = material.diffuse;
	const vec3 specularTexColor = material.specular;
	const vec3 fragNormal = normalize(fsNormal);
	const vec3 dirToLight = normalize(light.position - fsFragPos);

	/* Diffuse calculations */
	float diffuseStrength = max(dot(fragNormal, dirToLight), 0.0f);
	// vec3 diffuseColor = pointSLight.diffuse * (diffuseStrength * material.diffuse);
	vec3 diffuseColor = light.diffuse * (diffuseStrength * diffuseTexColor);

	/* Ambient calculations */
	vec3 ambientColor = material.ambient * light.ambient;

	/* Specular calculations */
	vec3 dirToCamera = normalize(viewPos - fsFragPos);
	vec3 reflectDir = reflect(-dirToLight, fragNormal); // reflect(I, S) expects the incident vector I to point into the surface defined by the normal vector S
	float specularResult = pow(max(dot(dirToCamera, reflectDir), 0.0f), material.shininess);
	vec3 specularColor = (specularTexColor * specularResult) * light.specular;

	/* Attenuation and final color */
	const float distToLight = length(light.position - fsFragPos);
	const float attenuation = 1.0f / (light.constant + (light.linear * distToLight) + (light.quadratic * distToLight * distToLight));
	vec3 resultColor = (diffuseColor + ambientColor + specularColor) * attenuation;
	
	return resultColor;
}

vec3 calcSpotLight(SLight light) 
{
	vec3 lightResult = calcLight(light);
	
	/* Calc falloff */
	const vec3 lightToFragmentDir = normalize(fsFragPos - light.position);
	const float cosAngleToFrag = dot(normalize(light.direction), lightToFragmentDir);
	const float cosInnerCutoff = cos(light.innerCutoff);
	const float cosOuterCutoff = cos(light.outerCutoff);
	const float deltaCutoff = cosInnerCutoff - cosOuterCutoff;
	const float intensity = clamp((cosAngleToFrag - cosOuterCutoff) / deltaCutoff, 0.0f, 1.0f);

	return lightResult * intensity;
}

vec3 calcDirLight() 
{
	SLight light;
	light.direction = normalize(vec3(-0.2f, -1.f, -0.3f));
	light.ambient = vec3(0.1f);
	light.diffuse = vec3(0.3f);
	light.specular = vec3(0.5f);
	light.constant = 1.f;
	light.linear = 0.0f;
	light.quadratic = 0.0f;

	// set light.position to FragPos - dirLight.direction so dirToLight (inside calcLight function) will result in -dirLight.direction
	light.position = fsFragPos - light.direction;
	return calcLight(light);
}

void main()
{
	vec3 dirLightColor = calcDirLight();
	vec3 pointLightColor = vec3(0.0f);
	for (int i = 0; i<MAX_POINT_LIGHTS; i++) 
	{
		pointLightColor += calcLight(pointLights[i]);
	}
	vec3 spotLightColor = calcSpotLight(spotLight);
	FragColor = fsColor * vec4(dirLightColor + pointLightColor + spotLightColor, 1.0f);
}
