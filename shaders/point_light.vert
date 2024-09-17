#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout(location = 0) out vec2 fragOffset;

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

layout(push_constant) uniform Push {
  vec4 position;
  vec4 color;
  float radius;
} push;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];

	//camera space computation
	vec4 lightCameraPos = ubo.view * vec4(push.position.xyz, 1.0);

	vec3 positionCamera = lightCameraPos.xyz +
		push.radius * fragOffset.x * vec3(1.0, 0.0, 0.0) +
		push.radius * fragOffset.y * vec3(0.0, 1.0, 0.0);

	gl_Position = ubo.projection * vec4(positionCamera, 1.0f);

	/*
	// world space computation
	vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	vec3 positionWorld = ubo.lightPosition.xyz
	+ LIGHT_RADIUS * fragOffset.x * cameraRightWorld
	+ LIGHT_RADIUS * fragOffset.y * cameraUpWorld;

	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
	*/
}