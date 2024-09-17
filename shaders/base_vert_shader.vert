#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projectionViewMatrix;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

layout(push_constant) uniform Push 
{
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main() {
	vec4 worldPos = push.modelMatrix * vec4(position, 1.0);
	
	gl_Position = ubo.projectionViewMatrix * worldPos;

	vec3 lightDirection = ubo.lightPosition - worldPos.xyz;

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	float attenuation = 1.0f / dot(lightDirection, lightDirection);

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 diffuseColor = lightColor * max(dot(normalWorldSpace, normalize(lightDirection)), 0);

	fragColor = (ambientColor + diffuseColor) * color;
}