#version 460 core

in vec3 fsNormal;
in vec3 fsFragPos;
in vec2 fsTexCoords;
in vec4 fsColor;

out vec4 FragColor;

// UBO
layout (binding = 0, std140) uniform SceneData {
	vec4 CameraPos;
	vec4 AmbientColor;
	vec4 SunlightDirection;
	vec4 SunlightColor;
	mat4 View;
	mat4 Proj;
	mat4 ViewProj;
} sceneData;

layout (binding = 1, std140) uniform PbrMaterial {
	vec4 ColorFactor;
	float MetalFactor;
	float RoughFactor;
	float AlphaCutoff;
	bool bColorBound;
	bool bMetalRoughBound;
} pbrMaterial;

layout (location = 1) uniform sampler2D ColorTex;
layout (location = 2) uniform sampler2D MetalRoughTex;
uniform bool ignoreLighting;

const float PI = 3.14159265359;


// BEGIN PBR FUNCTIONS
vec3 N;
vec3 V;
vec3 PbrF0;
vec3 PbrAlbedo;
vec3 PbrAmbientOcclusion;
float PbrMetalness;
float PbrRoughness;

float DistributionGGX(vec3 N, vec3 H)
{
	float a = PbrRoughness * PbrRoughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV)
{
	float r = (PbrRoughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV);
	float ggx1  = GeometrySchlickGGX(NdotL);
	
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

vec3 CalcReflectance(vec3 L, vec3 H, vec3 radiance) 
{
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H);        
	float G = GeometrySmith(N, V, L);      
	vec3 F = fresnelSchlick(max(dot(H, V), 0.0), PbrF0);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - PbrMetalness;
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular = numerator / denominator;  
		
	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);                
	vec3 Lo = (kD * PbrAlbedo / PI + specular) * radiance * NdotL; 
	return Lo;
}

vec3 CalcPointLight(vec3 lightPos, vec3 lightColor)
{
	vec3 fragToLight = lightPos - fsFragPos;
	vec3 L = normalize(fragToLight);
	vec3 H = normalize(V + L);
	float dist = length(fragToLight);
	float attenuation = 1.f / (dist * dist);
	vec3 radiance = lightColor * attenuation;
	vec3 result = CalcReflectance(L, H, radiance);
	return result;
}

vec3 CalcDirLight() 
{
	vec3 L = normalize(-sceneData.SunlightDirection.xyz);
	vec3 H = normalize(V + L);
	vec3 radiance = sceneData.SunlightColor.xyz;
	return CalcReflectance(L, H, radiance);
}

void InitPbr(vec4 srcColor) 
{
	PbrAlbedo = pow(srcColor.rgb, vec3(2.2f));
	N = normalize(fsNormal);
	V = normalize(sceneData.CameraPos.xyz - fsFragPos);
	PbrMetalness  = pbrMaterial.MetalFactor;
	PbrRoughness = pbrMaterial.RoughFactor;
	if (pbrMaterial.bMetalRoughBound) {
		vec4 metalRough = texture(MetalRoughTex, fsTexCoords);
		PbrMetalness *= metalRough.b;
		PbrRoughness *= metalRough.g;
	}
	PbrAmbientOcclusion = vec3(1.f); // temp

	PbrF0 = vec3(0.04); 
	PbrF0 = mix(PbrF0, PbrAlbedo, PbrMetalness);
}

vec3 CalcPbr() 
{
	vec3 ambient = vec3(0.03) * PbrAlbedo * PbrAmbientOcclusion;
	vec3 Lo = CalcDirLight();
	// TODO add point light calcs here
	vec3 color = ambient + Lo;
	
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));  
	return color;
}
// END PBR FUNCTIONS

void main()
{
	vec4 srcColor = pbrMaterial.ColorFactor;

	if (pbrMaterial.bColorBound)
		srcColor *= texture(ColorTex, fsTexCoords);

	if (srcColor.a * fsColor.a < pbrMaterial.AlphaCutoff)
		discard;

	if (ignoreLighting) {
		FragColor = srcColor * fsColor;
		return;
	}

	InitPbr(srcColor);
	vec3 result = CalcPbr();
	FragColor = vec4(result, 1.0) * fsColor;
}
