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

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

void main()
{
	vec3 lightDirection = ubo.lightPosition - fragWorldPos;

	float attenuation = 1.0f / dot(lightDirection, lightDirection);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseColor = lightColor * max(dot(normalize(fragNormal), normalize(lightDirection)), 0);

	outColor = vec4((ambientColor + diffuseColor) * fragColor, 1.0f);
}