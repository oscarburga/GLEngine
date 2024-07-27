#version 460 core

in VS_OUT {
	vec3 Normal;
	vec3 FragPos;
	vec2 TexCoords;
	vec4 Color;
	vec4 FragPosSunSpace;
	mat3 TBN;
} fs;

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
	mat4 LightSpaceTransform;
} sceneData;

layout (binding = 1, std140) uniform PbrMaterial {
	vec4 ColorFactor;
	float MetalFactor;
	float RoughFactor;
	float AlphaCutoff;
	float NormalScale;
	float OcclusionStrength;
	bool bColorBound;
	bool bMetalRoughBound;
	bool bNormalBound;
	bool bOcclusionBound;
} pbrMaterial;

layout (location = 1) uniform sampler2D ColorTex;
layout (location = 2) uniform sampler2D MetalRoughTex;
layout (location = 3) uniform sampler2D NormalTex;
layout (location = 4) uniform sampler2D OcclusionTex;
layout (location = 5) uniform sampler2D ShadowDepthTex;
layout (location = 32) uniform bool bIgnoreLighting;
layout (location = 33) uniform bool bShowDebugNormals;

const float PI = 3.14159265359;


// BEGIN PBR FUNCTIONS
vec3 N;
vec3 V;
vec3 PbrF0;
vec3 PbrAlbedo;
float PbrMetalness;
float PbrRoughness;
float PbrAmbientOcclusion;

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
	vec3 fragToLight = lightPos - fs.FragPos;
	vec3 L = normalize(fragToLight);
	vec3 H = normalize(V + L);
	float dist = length(fragToLight);
	float attenuation = 1.f / (dist * dist);
	vec3 radiance = lightColor * attenuation;
	vec3 result = CalcReflectance(L, H, radiance);
	return result;
}

float DirLightShadowFactor() 
{
	vec3 projCoords = fs.FragPosSunSpace.xyz / fs.FragPosSunSpace.w;
	projCoords = projCoords * 0.5f + 0.5f;

	vec3 sunlightDir = normalize(-sceneData.SunlightDirection.xyz);
	float bias = max(0.05f * (1.0f - dot(N, sunlightDir)), 0.005f);
	float curDepth = projCoords.z; // depth of this fragment

	// simple PCF
	float shadow = 0.0f;
	vec2 texelSize = 1.0 / textureSize(ShadowDepthTex, 0);
	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			vec2 pcfCoords = projCoords.xy + vec2(x, y) * texelSize;
			float pcfDepth = texture(ShadowDepthTex, pcfCoords).r;
			shadow += float(((curDepth - bias) > pcfDepth)); 
		}
	}
	shadow *= float(projCoords.z <= 1.0f) / 9.0f;
	// float closestDepth = texture(ShadowDepthTex, projCoords.xy).r; // closest to sun
	// float curDepth = projCoords.z; // depth of this fragment
	// float shadow = float( ((curDepth - bias) > closestDepth) && (projCoords.z <= 1.0f) ); 
	return 1.0f - shadow;
}

vec3 CalcDirLight() 
{
	vec3 L = normalize(-sceneData.SunlightDirection.xyz);
	vec3 H = normalize(V + L);
	vec3 radiance = sceneData.SunlightColor.xyz * sceneData.SunlightDirection.w;
	float shadowFactor = DirLightShadowFactor();
	return CalcReflectance(L, H, radiance) * shadowFactor;
}

void InitPbr(vec4 srcColor) 
{
	PbrAlbedo = pow(srcColor.rgb, vec3(2.2f));
	N = normalize(fs.Normal); 
	if (pbrMaterial.bNormalBound) {
		N = texture(NormalTex, fs.TexCoords).rgb;
		N = (N * 2.f) - 1.f;
		N = normalize(fs.TBN * N);
	}
	V = normalize(sceneData.CameraPos.xyz - fs.FragPos);
	PbrMetalness  = pbrMaterial.MetalFactor;
	PbrRoughness = pbrMaterial.RoughFactor;
	if (pbrMaterial.bMetalRoughBound) {
		vec4 metalRough = texture(MetalRoughTex, fs.TexCoords);
		PbrMetalness *= metalRough.b;
		PbrRoughness *= metalRough.g;
	}
	PbrAmbientOcclusion = 1.f; // temp
	if (pbrMaterial.bOcclusionBound) {
		PbrAmbientOcclusion = pbrMaterial.OcclusionStrength * texture(OcclusionTex, fs.TexCoords).r;
	}

	PbrF0 = mix(vec3(0.04), PbrAlbedo, PbrMetalness);
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
	vec4 srcColor = pbrMaterial.ColorFactor * fs.Color;

	if (pbrMaterial.bColorBound)
		srcColor *= texture(ColorTex, fs.TexCoords);

	if (srcColor.a < pbrMaterial.AlphaCutoff)
		discard;

	if (bIgnoreLighting) {
		FragColor = srcColor;
		return;
	}

	InitPbr(srcColor);

	if (bShowDebugNormals) {
		FragColor = vec4(N, srcColor.a);
		return;
	}

	vec3 result = CalcPbr();
	FragColor = vec4(result, srcColor.a) * fs.Color;
}
