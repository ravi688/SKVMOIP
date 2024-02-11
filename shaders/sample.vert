#version 450

/* index buffer: { 0, 1, 2, 2, 3, 0 } */

const vec2 vertCoords[6] = 
{
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, -1.0)
};

const vec2 texCoords[6] = 
{
	vec2(0.0, 0.0),
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 1.0),
	vec2(1.0, 0.0),
	vec2(0.0, 0.0)
};

layout(location = 0) out vec2 outTexCoord;

void main()
{
// 0 1 2 3 4 5
// 0 1 2 2 3 0
	gl_Position = vec4(vertCoords[gl_VertexIndex], 1.0, 1.0);
	outTexCoord = texCoords[gl_VertexIndex];
}
