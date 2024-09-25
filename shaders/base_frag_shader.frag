#version 450

layout (location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragWorldPos;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec2 fragUV;

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
	mat4 inverseView;
	vec4 ambientLightColor;
	PointLight pointLights[10]; // use specialisation constants of Vulkan
	int numLights;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
	vec3 diffuseColor = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
	vec3 specularColor = vec3(0.0);
	vec3 surfaceNormal = normalize(fragNormal);
	vec3 cameraPos = ubo.inverseView[3].xyz;
	vec3 viewDirection = normalize(cameraPos - fragWorldPos);

	for (int i = 0; i < ubo.numLights; ++i)
	{
		vec3 lightDirection = ubo.pointLights[i].position.xyz - fragWorldPos;
		vec3 reflectDirection = reflect(-lightDirection, surfaceNormal);
		float cosAngIncidence = max(dot(surfaceNormal, lightDirection), 0);

		float attenuation = 1.0f / dot(lightDirection, lightDirection);

		float phongTerm = dot(reflectDirection, viewDirection);
		phongTerm = clamp(phongTerm, 0, 1);
		phongTerm = cosAngIncidence == 0.0 ? 0.0: phongTerm;
		phongTerm = pow(phongTerm, 400.0);

		vec3 halfAngle = normalize(lightDirection + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 512.0);

		vec3 lightColor = ubo.pointLights[i].color.xyz 
			* ubo.pointLights[i].color.w 
			* attenuation;
		
		diffuseColor += lightColor * cosAngIncidence;
		specularColor += lightColor * blinnTerm;
	}

	outColor = texture(texSampler, fragUV);
	
	//outColor = vec4(diffuseColor * fragColor + specularColor * fragColor, 1.0f);
}