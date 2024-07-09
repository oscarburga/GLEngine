#version 460 core
#extension GL_EXT_scalar_block_layout : require

in vec3 fsNormal;
in vec3 fsFragPos;
in vec2 fsTexCoords;
in vec4 fsColor;

out vec4 FragColor;

struct SMaterial {
	// diffuse map texture. Will also use this for ambient color, but could choose to use a separate ambient texture/vector
	sampler2D diffuseMap; 
	sampler2D specularMap;
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

// UBO
layout (binding = 0, std430) uniform SceneData {
	vec4 CameraPos;
	vec4 AmbientColor;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
} sceneData;

uniform SMaterial material;
uniform SLight pointLights[MAX_POINT_LIGHTS];
uniform SLight spotLight;
uniform bool ignoreLighting;

vec3 calcLight(SLight light) 
{
	if (light.constant == 0.0f) 
		return vec3(0.0f);
	const vec3 diffuseTexColor = texture(material.diffuseMap, fsTexCoords).xyz;
	const vec3 specularTexColor = texture(material.specularMap, fsTexCoords).xyz;
	const vec3 fragNormal = normalize(fsNormal);
	const vec3 dirToLight = normalize(light.position - fsFragPos);

	/* Diffuse calculations */
	float diffuseStrength = max(dot(fragNormal, dirToLight), 0.0f);
	// vec3 diffuseColor = pointSLight.diffuse * (diffuseStrength * material.diffuse);
	vec3 diffuseColor = light.diffuse * (diffuseStrength * diffuseTexColor);

	/* Ambient calculations */
	// vec3 ambientColor = material.ambient * pointSLight.ambient;
	vec3 ambientColor = diffuseTexColor * light.ambient;

	/* Specular calculations */
	vec3 dirToCamera = normalize(sceneData.CameraPos.xyz - fsFragPos);
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
	if (ignoreLighting) {
		FragColor = fsColor;
		return;
	}
	vec3 dirLightColor = calcDirLight();
	vec3 pointLightColor = vec3(0.0f);
	for (int i = 0; i<MAX_POINT_LIGHTS; i++) 
	{
		pointLightColor += calcLight(pointLights[i]);
	}
	vec3 spotLightColor = calcSpotLight(spotLight);
	FragColor = fsColor * vec4(dirLightColor + pointLightColor + spotLightColor, 1.0f);
}
