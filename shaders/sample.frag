#version 450

layout(set = 0, binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;

void main()
{
	//outColor = texture2D(texSampler, inTexCoord);
	outColor = vec3(0.5, 0.7, 0.8);
}
