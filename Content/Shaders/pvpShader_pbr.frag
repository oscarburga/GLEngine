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
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a      = roughness*roughness;
	float a2     = a*a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num   = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float num   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2  = GeometrySchlickGGX(NdotV, roughness);
	float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
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

	vec3 albedo     = pow(srcColor.rgb, vec3(2.2f));
	vec3 normal     = normalize(fsNormal);
	float metallic  = pbrMaterial.MetalFactor;
	float roughness = pbrMaterial.RoughFactor;
	if (pbrMaterial.bMetalRoughBound) {
		vec4 metalRough = texture(MetalRoughTex, fsTexCoords);
		metallic *= metalRough.b;
		roughness *= metalRough.g;
	}
	float ao        = 1.f; // temp


	vec3 N = normalize(fsNormal);
	vec3 V = normalize(sceneData.CameraPos.xyz - fsFragPos);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
			   
	// reflectance equation
	vec3 Lo = vec3(0.0);
	// calculate per-light radiance
	vec3 L = normalize(-sceneData.SunlightDirection.xyz);
	vec3 H = normalize(V + L);
	float attenuation = 1.0;
	vec3 radiance     = sceneData.SunlightColor.xyz;//  * sceneData.SunlightDirection.w;
	
	// cook-torrance brdf
	float NDF = DistributionGGX(N, H, roughness);        
	float G   = GeometrySmith(N, V, L, roughness);      
	vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);       
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;	  
	
	vec3 numerator    = NDF * G * F;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	vec3 specular     = numerator / denominator;  
		
	// add to outgoing radiance Lo
	float NdotL = max(dot(N, L), 0.0);                
	Lo += (kD * albedo / PI + specular) * radiance * NdotL; 
  
	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;
	
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));  
   
	FragColor = vec4(color, 1.0);
}
