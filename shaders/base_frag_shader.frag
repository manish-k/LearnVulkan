#version 450

layout (location = 0) out vec4 outColor;
layout(push_constant) uniform PushConstant
{
	vec2 offset;
	vec3 color;
} pConstant;

void main()
{
	outColor = vec4(pConstant.color, 1.0);
}