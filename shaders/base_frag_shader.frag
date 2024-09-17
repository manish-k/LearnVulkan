#version 450

layout (location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform Push 
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

struct PointLight
{
	vec4 position;
	vec4 color; //w is for intensity
};

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor;
	PointLight pointLights[10]; // use specialisation constants of Vulkan
	int numLights;
} ubo;

void main()
{
	vec3 diffuseColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 surfaceNormal = normalize(fragNormal);

	for (int i = 0; i < ubo.numLights; ++i)
	{
		vec3 lightDirection = ubo.pointLights[i].position.xyz - fragWorldPos;

		float attenuation = 1.0f / dot(lightDirection, lightDirection);

		vec3 lightColor = ubo.pointLights[i].color.xyz 
			* ubo.pointLights[i].color.w 
			* attenuation;
		diffuseColor += lightColor 
			* max(dot(surfaceNormal, normalize(lightDirection)), 0);
	}
	
	outColor = vec4(diffuseColor * fragColor, 1.0f);
}