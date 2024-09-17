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

layout(set = 0, binding = 0) uniform GlobalUbo
{
	mat4 projection;
	mat4 view;
	vec4 ambientLightColor;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;

const float LIGHT_RADIUS = 0.05;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];

	//camera space computation
	vec4 lightCameraPos = ubo.view * vec4(ubo.lightPosition, 1.0);

	vec3 positionCamera = lightCameraPos.xyz +
		LIGHT_RADIUS * fragOffset.x * vec3(1.0, 0.0, 0.0) +
		LIGHT_RADIUS * fragOffset.y * vec3(0.0, 1.0, 0.0);

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